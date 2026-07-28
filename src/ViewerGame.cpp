#include "ViewerGame.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Input/ButtonState.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"

namespace CnaGltfViewer
{
    namespace
    {
        namespace fs = std::filesystem;

        using Microsoft::Xna::Framework::MathHelper;
        using Microsoft::Xna::Framework::Matrix;
        using Microsoft::Xna::Framework::Vector3;

        struct SceneBounds
        {
            bool hasPoints = false;
            float minimumX = std::numeric_limits<float>::max();
            float minimumY = std::numeric_limits<float>::max();
            float minimumZ = std::numeric_limits<float>::max();
            float maximumX = std::numeric_limits<float>::lowest();
            float maximumY = std::numeric_limits<float>::lowest();
            float maximumZ = std::numeric_limits<float>::lowest();

            void AddPoint(const float x, const float y, const float z)
            {
                if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
                {
                    return;
                }

                hasPoints = true;
                minimumX = std::min(minimumX, x);
                minimumY = std::min(minimumY, y);
                minimumZ = std::min(minimumZ, z);
                maximumX = std::max(maximumX, x);
                maximumY = std::max(maximumY, y);
                maximumZ = std::max(maximumZ, z);
            }
        };

        [[nodiscard]] std::string ReadTextFile(const fs::path& path)
        {
            std::ifstream input(path);
            if (!input)
            {
                throw std::runtime_error("Could not read generated Model CNJ file: " + path.string());
            }
            return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
        }

        void AddMeshBounds(const fs::path& vertexPath, const int stride, SceneBounds& bounds)
        {
            if (stride < static_cast<int>(sizeof(float) * 3))
            {
                throw std::runtime_error("Generated CNJ vertex stride is too small: " + vertexPath.string());
            }

            std::ifstream input(vertexPath, std::ios::binary);
            if (!input)
            {
                throw std::runtime_error("Could not read generated vertex sidecar: " + vertexPath.string());
            }
            const std::vector<char> bytes{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
            if (bytes.size() % static_cast<std::size_t>(stride) != 0)
            {
                throw std::runtime_error("Generated vertex sidecar has an invalid stride: " + vertexPath.string());
            }

            for (std::size_t offset = 0; offset < bytes.size(); offset += static_cast<std::size_t>(stride))
            {
                float position[3]{};
                std::memcpy(position, bytes.data() + offset, sizeof(position));
                bounds.AddPoint(position[0], position[1], position[2]);
            }
        }

        void AddModelBounds(const fs::path& directory, const fs::path& asset, SceneBounds& bounds)
        {
            const fs::path cnjPath = directory / (asset.string() + ".cnj");
            const std::string json = ReadTextFile(cnjPath);
            const std::regex meshExpression(
                R"REGEX("vertices"\s*:\s*"([^"]+)"\s*,\s*"indices"\s*:\s*"[^"]+"\s*,\s*"vertexStride"\s*:\s*([0-9]+))REGEX");

            for (std::sregex_iterator it(json.begin(), json.end(), meshExpression), end; it != end; ++it)
            {
                const fs::path vertices = directory / (*it)[1].str();
                AddMeshBounds(vertices, std::stoi((*it)[2].str()), bounds);
            }
        }
    }

    ViewerGame::ViewerGame(ViewerOptions options)
        : options_(std::move(options)),
          graphics_(this)
    {
        getWindowProperty().setTitleProperty("CNA glTF View");
        graphics_.setPreferredBackBufferWidthProperty(1280);
        graphics_.setPreferredBackBufferHeightProperty(720);
    }

    void ViewerGame::LoadContent()
    {
        convertedScene_ = CnjConverter::Convert(options_);
        getContentProperty().setRootDirectoryProperty(convertedScene_->outputDirectory.string());

        models_.reserve(convertedScene_->modelAssets.size());
        for (const fs::path& asset : convertedScene_->modelAssets)
        {
            models_.push_back(getContentProperty().Load<Microsoft::Xna::Framework::Graphics::Model>(asset.string()));
        }

        ConfigureCameraFromConvertedModels();
    }

    void ViewerGame::Update(Microsoft::Xna::Framework::GameTime& gameTime)
    {
        (void)gameTime;

        using namespace Microsoft::Xna::Framework::Input;
        const KeyboardState keyboard = Keyboard::GetState();
        if (keyboard.IsKeyDown(Keys::Escape))
        {
            Exit();
            return;
        }
        if (keyboard.IsKeyDown(Keys::R))
        {
            ResetCamera();
        }

        const MouseState currentMouse = Mouse::GetState();
        if (previousMouse_.has_value())
        {
            const MouseState& previousMouse = *previousMouse_;
            if (currentMouse.getLeftButtonProperty() == ButtonState::Pressed &&
                previousMouse.getLeftButtonProperty() == ButtonState::Pressed)
            {
                const int deltaX = currentMouse.getXProperty() - previousMouse.getXProperty();
                const int deltaY = currentMouse.getYProperty() - previousMouse.getYProperty();
                yaw_ -= static_cast<float>(deltaX) * 0.01f;
                pitch_ = MathHelper::Clamp(pitch_ - static_cast<float>(deltaY) * 0.01f, -1.45f, 1.45f);
            }

            const int wheelDelta = currentMouse.getScrollWheelValueProperty() -
                                   previousMouse.getScrollWheelValueProperty();
            if (wheelDelta != 0)
            {
                distance_ *= std::exp(-static_cast<float>(wheelDelta) / 1000.0f);
                const float minimumDistance = std::max(sceneRadius_ * 0.05f, 0.01f);
                const float maximumDistance = std::max(sceneRadius_ * 1000.0f, 10.0f);
                distance_ = MathHelper::Clamp(distance_, minimumDistance, maximumDistance);
            }
        }
        previousMouse_ = currentMouse;
    }

    void ViewerGame::Draw(const Microsoft::Xna::Framework::GameTime& gameTime)
    {
        (void)gameTime;

        using namespace Microsoft::Xna::Framework;
        using namespace Microsoft::Xna::Framework::Graphics;

        GraphicsDevice& device = getGraphicsDeviceProperty();
        device.Clear(Color(24, 29, 38, 255));
        device.setDepthStencilStateProperty(DepthStencilState::Default);
        device.setRasterizerStateProperty(RasterizerState::CullNone);

        const float horizontalDistance = std::cos(pitch_) * distance_;
        const Vector3 cameraPosition(
            target_.X + std::sin(yaw_) * horizontalDistance,
            target_.Y + std::sin(pitch_) * distance_,
            target_.Z + std::cos(yaw_) * horizontalDistance);
        const Matrix view = Matrix::CreateLookAt(cameraPosition, target_, Vector3::Up);

        const Viewport viewport = device.getViewportProperty();
        const float aspect = static_cast<float>(viewport.getWidthProperty()) /
                             static_cast<float>(std::max(viewport.getHeightProperty(), 1));
        const float farPlane = std::max(distance_ + sceneRadius_ * 4.0f, 100.0f);
        const Matrix projection = Matrix::CreatePerspectiveFieldOfView(
            MathHelper::PiOver4, aspect, 0.01f, farPlane);

        for (Microsoft::Xna::Framework::Graphics::Model& model : models_)
        {
            model.Draw(Matrix::getIdentityProperty(), view, projection);
        }
    }

    void ViewerGame::ResetCamera()
    {
        yaw_ = 0.7f;
        pitch_ = 0.35f;
        distance_ = std::max(sceneRadius_ * 2.4f, 1.5f);
    }

    void ViewerGame::ConfigureCameraFromConvertedModels()
    {
        if (!convertedScene_.has_value())
        {
            return;
        }

        SceneBounds bounds;
        for (const fs::path& asset : convertedScene_->modelAssets)
        {
            AddModelBounds(convertedScene_->outputDirectory, asset, bounds);
        }

        if (bounds.hasPoints)
        {
            target_ = Vector3(
                (bounds.minimumX + bounds.maximumX) * 0.5f,
                (bounds.minimumY + bounds.maximumY) * 0.5f,
                (bounds.minimumZ + bounds.maximumZ) * 0.5f);
            const float halfX = (bounds.maximumX - bounds.minimumX) * 0.5f;
            const float halfY = (bounds.maximumY - bounds.minimumY) * 0.5f;
            const float halfZ = (bounds.maximumZ - bounds.minimumZ) * 0.5f;
            sceneRadius_ = std::max(std::sqrt(halfX * halfX + halfY * halfY + halfZ * halfZ), 0.1f);
        }
        else
        {
            target_ = Vector3::Zero;
            sceneRadius_ = 1.0f;
        }

        ResetCamera();
    }

    GetTypeNameCPP(ViewerGame, "CnaGltfViewer.ViewerGame")
}
