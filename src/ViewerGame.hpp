#pragma once

#include "CnjConversion.hpp"
#include "CommandLine.hpp"

#include <optional>
#include <vector>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Input/MouseState.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"

namespace CnaGltfViewer
{
    class ViewerGame final : public Microsoft::Xna::Framework::Game
    {
    public:
        explicit ViewerGame(ViewerOptions options);

        GetTypeNameHPP()

    protected:
        void LoadContent() override;
        void Update(Microsoft::Xna::Framework::GameTime& gameTime) override;
        void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

    private:
        void ResetCamera();
        void ConfigureCameraFromConvertedModels();

        ViewerOptions options_;
        Microsoft::Xna::Framework::GraphicsDeviceManager graphics_;
        std::optional<ConvertedScene> convertedScene_;
        std::vector<Microsoft::Xna::Framework::Graphics::Model> models_;
        std::optional<Microsoft::Xna::Framework::Input::MouseState> previousMouse_;
        Microsoft::Xna::Framework::Vector3 target_;
        float sceneRadius_ = 1.0f;
        float yaw_ = 0.7f;
        float pitch_ = 0.35f;
        float distance_ = 3.0f;
    };
}
