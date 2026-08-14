#include "ViewerGame.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Graphics/AlphaModeEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Effect.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/IEffectLights.hpp"
#include "Microsoft/Xna/Framework/Graphics/IEffectMatrices.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/PbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedPbrEffect.hpp"
#include "Microsoft/Xna/Framework/Input/ButtonState.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"
#include "System/TimeSpan.hpp"

namespace CnaGltfViewer
{
    namespace
    {
        namespace fs = std::filesystem;

        using Microsoft::Xna::Framework::BoundingSphere;
        using Microsoft::Xna::Framework::Color;
        using Microsoft::Xna::Framework::MathHelper;
        using Microsoft::Xna::Framework::Matrix;
        using Microsoft::Xna::Framework::Rectangle;
        using Microsoft::Xna::Framework::Vector3;
        using namespace Microsoft::Xna::Framework::Graphics;

        using Glyph = std::array<unsigned char, 7>;

        [[nodiscard]] Glyph PixelGlyph(const char value)
        {
            switch (static_cast<char>(std::toupper(static_cast<unsigned char>(value))))
            {
                case 'A': return {14, 17, 17, 31, 17, 17, 17};
                case 'B': return {30, 17, 17, 30, 17, 17, 30};
                case 'C': return {14, 17, 16, 16, 16, 17, 14};
                case 'D': return {30, 17, 17, 17, 17, 17, 30};
                case 'E': return {31, 16, 16, 30, 16, 16, 31};
                case 'F': return {31, 16, 16, 30, 16, 16, 16};
                case 'G': return {14, 17, 16, 23, 17, 17, 15};
                case 'H': return {17, 17, 17, 31, 17, 17, 17};
                case 'I': return {31, 4, 4, 4, 4, 4, 31};
                case 'J': return {7, 2, 2, 2, 18, 18, 12};
                case 'K': return {17, 18, 20, 24, 20, 18, 17};
                case 'L': return {16, 16, 16, 16, 16, 16, 31};
                case 'M': return {17, 27, 21, 21, 17, 17, 17};
                case 'N': return {17, 25, 21, 19, 17, 17, 17};
                case 'O': return {14, 17, 17, 17, 17, 17, 14};
                case 'P': return {30, 17, 17, 30, 16, 16, 16};
                case 'Q': return {14, 17, 17, 17, 21, 18, 13};
                case 'R': return {30, 17, 17, 30, 20, 18, 17};
                case 'S': return {15, 16, 16, 14, 1, 1, 30};
                case 'T': return {31, 4, 4, 4, 4, 4, 4};
                case 'U': return {17, 17, 17, 17, 17, 17, 14};
                case 'V': return {17, 17, 17, 17, 17, 10, 4};
                case 'W': return {17, 17, 17, 21, 21, 21, 10};
                case 'X': return {17, 17, 10, 4, 10, 17, 17};
                case 'Y': return {17, 17, 10, 4, 4, 4, 4};
                case 'Z': return {31, 1, 2, 4, 8, 16, 31};
                case '0': return {14, 17, 19, 21, 25, 17, 14};
                case '1': return {4, 12, 4, 4, 4, 4, 14};
                case '2': return {14, 17, 1, 2, 4, 8, 31};
                case '3': return {30, 1, 1, 14, 1, 1, 30};
                case '4': return {2, 6, 10, 18, 31, 2, 2};
                case '5': return {31, 16, 16, 30, 1, 1, 30};
                case '6': return {14, 16, 16, 30, 17, 17, 14};
                case '7': return {31, 1, 2, 4, 8, 8, 8};
                case '8': return {14, 17, 17, 14, 17, 17, 14};
                case '9': return {14, 17, 17, 15, 1, 1, 14};
                case '-': return {0, 0, 0, 31, 0, 0, 0};
                case '_': return {0, 0, 0, 0, 0, 0, 31};
                case ':': return {0, 4, 4, 0, 4, 4, 0};
                case '.': return {0, 0, 0, 0, 0, 6, 6};
                case '/': return {1, 1, 2, 4, 8, 16, 16};
                case '+': return {0, 4, 4, 31, 4, 4, 0};
                case '!': return {4, 4, 4, 4, 4, 0, 4};
                case ' ': return {0, 0, 0, 0, 0, 0, 0};
                default:  return {14, 17, 1, 2, 4, 0, 4};
            }
        }

        void DrawPixelText(SpriteBatch& batch, const Texture2D& pixel,
                           const int x, const int y, const std::string_view text,
                           const Color color)
        {
            constexpr int scale = 2;
            constexpr int advance = 6 * scale;
            int cursorX = x;
            for (const char character : text)
            {
                const Glyph glyph = PixelGlyph(character);
                for (int row = 0; row != 7; ++row)
                {
                    for (int column = 0; column != 5; ++column)
                    {
                        if ((glyph[static_cast<std::size_t>(row)] & (1U << (4 - column))) != 0)
                        {
                            batch.Draw(pixel,
                                       Rectangle(cursorX + column * scale, y + row * scale,
                                                 scale, scale),
                                       color);
                        }
                    }
                }
                cursorX += advance;
            }
        }

        [[nodiscard]] const char* DiagnosticKindName(const GltfImportDiagnosticKindEXT kind)
        {
            switch (kind)
            {
                case GltfImportDiagnosticKindEXT::Information: return "information";
                case GltfImportDiagnosticKindEXT::GeneratedData: return "generated";
                case GltfImportDiagnosticKindEXT::InvalidSourceData: return "invalid-source";
                case GltfImportDiagnosticKindEXT::Approximation: return "approximation";
                case GltfImportDiagnosticKindEXT::DroppedData: return "dropped";
                case GltfImportDiagnosticKindEXT::UnsupportedFeature: return "unsupported";
            }
            return "unknown";
        }

        [[nodiscard]] bool IsDoubleSided(const Effect& effect)
        {
            if (const auto* pbr = dynamic_cast<const PbrEffect*>(&effect))
            {
                return pbr->getDoubleSidedEXTProperty();
            }
            if (const auto* skinned = dynamic_cast<const SkinnedPbrEffect*>(&effect))
            {
                return skinned->getDoubleSidedEXTProperty();
            }
            return false;
        }

        [[nodiscard]] AlphaModeEXT AlphaMode(const Effect& effect)
        {
            if (const auto* pbr = dynamic_cast<const PbrEffect*>(&effect))
            {
                return pbr->getAlphaModeEXTProperty();
            }
            if (const auto* skinned = dynamic_cast<const SkinnedPbrEffect*>(&effect))
            {
                return skinned->getAlphaModeEXTProperty();
            }
            return AlphaModeEXT::Opaque;
        }

        [[nodiscard]] bool IsParkedSkinnedUnlit(SkinnedEffect& effect)
        {
            const Vector3 ambient = effect.getAmbientLightColorProperty();
            return ambient.X == 1.0f && ambient.Y == 1.0f && ambient.Z == 1.0f &&
                   !effect.getDirectionalLight0Property().getEnabledProperty() &&
                   !effect.getDirectionalLight1Property().getEnabledProperty() &&
                   !effect.getDirectionalLight2Property().getEnabledProperty();
        }

        void ApplySkinPalette(const ModelSkinEXT& skin, const std::vector<Matrix>& transforms)
        {
            for (ModelMesh* mesh : skin.Meshes)
            {
                if (mesh == nullptr)
                {
                    continue;
                }
                for (ModelMeshPart* part : mesh->getMeshPartsProperty())
                {
                    Effect* effect = part != nullptr ? part->getEffectProperty() : nullptr;
                    if (auto* skinned = dynamic_cast<SkinnedEffect*>(effect))
                    {
                        skinned->SetBoneTransforms(transforms);
                    }
                    else if (auto* skinnedPbr = dynamic_cast<SkinnedPbrEffect*>(effect))
                    {
                        skinnedPbr->SetBoneTransforms(transforms);
                    }
                }
            }
        }

        [[nodiscard]] std::string JoinClipNames(const std::set<std::string>& names)
        {
            if (names.empty())
            {
                return "none";
            }

            std::ostringstream result;
            bool first = true;
            for (const std::string& name : names)
            {
                if (!first)
                {
                    result << ", ";
                }
                result << name;
                first = false;
            }
            return result.str();
        }
    }

    ViewerGame::ViewerGame(ViewerOptions options)
        : options_(std::move(options)),
          graphics_(this)
    {
        getWindowProperty().setTitleProperty("CNA glTF Viewer");
        // Match CNA's initial desktop window size. This also keeps an Xvfb capture exact: a bare
        // X11 server has no window manager to acknowledge a later asynchronous resize request.
        graphics_.setPreferredBackBufferWidthProperty(800);
        graphics_.setPreferredBackBufferHeightProperty(480);
        graphics_.setPreferredPresentationModeProperty(
            Microsoft::Xna::Framework::PresentationMode::NativeBackBuffer);
    }

    ViewerGame::~ViewerGame()
    {
        if (!convertedScene_.has_value() || !convertedScene_->temporaryOutput)
        {
            return;
        }

        std::error_code error;
        const std::uintmax_t removed = fs::remove_all(convertedScene_->outputDirectory, error);
        if (error)
        {
            std::cerr << "warning: could not remove temporary CNJ directory "
                      << convertedScene_->outputDirectory << ": " << error.message() << "\n";
        }
        else
        {
            std::cout << "Removed temporary CNJ directory "
                      << convertedScene_->outputDirectory << " (" << removed
                      << " filesystem entries).\n";
        }
    }

    void ViewerGame::LoadContent()
    {
        LoadModels();
        ConfigureFallbackLighting();
        ConfigureAnimations();
        ConfigureDiagnostics();
        ConfigureCameraFromModels();

        GraphicsDevice& device = getGraphicsDeviceProperty();
        spriteBatch_ = std::make_unique<SpriteBatch>(device);
        overlayPixel_ = std::make_unique<Texture2D>(device, 1, 1);
        const Color white = Color::White;
        overlayPixel_->SetData(&white, 1);
    }

    void ViewerGame::LoadModels()
    {
        std::error_code error;
        const fs::path input = fs::absolute(options_.inputPath, error);
        if (error || !fs::is_regular_file(input, error))
        {
            throw std::runtime_error(
                "Input glTF file does not exist or is not a regular file: " +
                options_.inputPath.string());
        }

        if (options_.direct)
        {
            getContentProperty().setRootDirectoryProperty(input.parent_path().string());
            models_.reserve(1);
            models_.push_back(getContentProperty().Load<Model>(input.filename().string()));
            std::cout << "Loaded glTF directly through ContentManager: " << input << "\n";
            return;
        }

        convertedScene_ = CnjConverter::Convert(options_);
        getContentProperty().setRootDirectoryProperty(convertedScene_->outputDirectory.string());

        models_.reserve(convertedScene_->modelAssets.size());
        for (const fs::path& asset : convertedScene_->modelAssets)
        {
            models_.push_back(getContentProperty().Load<Model>(asset.string()));
        }
    }

    void ViewerGame::ConfigureFallbackLighting()
    {
        const bool importedAnyLight = std::any_of(
            models_.begin(), models_.end(),
            [](const Model& model)
            {
                return model.getGltfImportReportEXTProperty().ImportedLightCount != 0;
            });
        if (importedAnyLight)
        {
            return;
        }

        std::unordered_set<IEffectLights*> configured;
        for (Model& model : models_)
        {
            for (ModelMesh* mesh : model.getMeshesProperty())
            {
                for (Effect* effect : mesh->getEffectsPropertyMutable())
                {
                    auto* lights = dynamic_cast<IEffectLights*>(effect);
                    if (lights == nullptr || !lights->getLightingEnabledProperty())
                    {
                        continue;
                    }

                    // KHR_materials_unlit maps to LightingEnabled=false on BasicEffect. Real XNA's
                    // SkinnedEffect has no such flag, so CNA represents the same semantic with an
                    // all-white ambient term and three parked lights. Preserve that authored state.
                    if (auto* skinned = dynamic_cast<SkinnedEffect*>(effect);
                        skinned != nullptr && IsParkedSkinnedUnlit(*skinned))
                    {
                        continue;
                    }
                    if (!configured.insert(lights).second)
                    {
                        continue;
                    }
                    lights->EnableDefaultLighting();
                }
            }
        }

        std::cout << "No imported lights; enabled default lighting on " << configured.size()
                  << " effect(s).\n";
    }

    void ViewerGame::ConfigureAnimations()
    {
        std::set<std::string> availableNames;
        bool selectedClipFound = false;

        for (Model& model : models_)
        {
            for (const ModelSkinEXT& skin : model.getSkinsEXTProperty())
            {
                if (skin.Data == nullptr)
                {
                    continue;
                }
                for (const auto& [name, clip] : skin.Data->AnimationClips)
                {
                    (void)clip;
                    availableNames.insert(name);
                }

                SkinPlayback playback;
                playback.skin = &skin;
                playback.player = std::make_unique<AnimationPlayer>(*skin.Data);

                if (options_.clipName.has_value())
                {
                    const auto clip = skin.Data->AnimationClips.find(*options_.clipName);
                    if (clip != skin.Data->AnimationClips.end())
                    {
                        playback.player->StartClip(clip->second);
                        selectedClipFound = true;
                    }
                }
                playback.player->Update(System::TimeSpan::Zero, false, true);
                ApplySkinPalette(skin, playback.player->GetSkinTransforms());
                skinPlaybacks_.push_back(std::move(playback));
            }

            if (auto* animations = dynamic_cast<ModelAnimationsEXT*>(model.getTagProperty()))
            {
                for (const auto& [name, clip] : animations->Clips)
                {
                    availableNames.insert(name);
                    if (options_.clipName.has_value() && name == *options_.clipName)
                    {
                        ApplyClipToBonesEXT(model, clip, System::TimeSpan::Zero);
                        rigidPlaybacks_.push_back({.model = &model, .clip = &clip});
                        selectedClipFound = true;
                    }
                }
            }
        }

        if (options_.clipName.has_value() && !selectedClipFound)
        {
            throw std::runtime_error(
                "Animation clip '" + *options_.clipName +
                "' was not found. Available clips: " + JoinClipNames(availableNames) + ".");
        }

        if (options_.clipName.has_value())
        {
            std::cout << "Playing animation clip '" << *options_.clipName << "'.\n";
        }
        else
        {
            std::cout << "Showing bind/static pose. Available clips: "
                      << JoinClipNames(availableNames) << ".\n";
        }
    }

    void ViewerGame::ConfigureDiagnostics()
    {
        std::size_t warnings = 0;
        std::size_t dropped = 0;
        std::size_t approximated = 0;
        std::size_t actionable = 0;

        for (std::size_t modelIndex = 0; modelIndex != models_.size(); ++modelIndex)
        {
            const GltfImportReportEXT& report =
                models_[modelIndex].getGltfImportReportEXTProperty();
            warnings += report.getWarningCountProperty();
            dropped += report.getDroppedFeatureCountProperty();
            approximated += report.getApproximationCountProperty();

            std::cout << "glTF import report [model " << modelIndex << "]: "
                      << report.NodeCount << " nodes, " << report.PrimitiveCount
                      << " primitives, " << report.getWarningCountProperty() << " warning(s).\n";
            for (const GltfImportDiagnosticEXT& diagnostic : report.Diagnostics)
            {
                std::cout << "  "
                          << (diagnostic.Severity == GltfImportDiagnosticSeverityEXT::Warning
                                  ? "warning" : "info")
                          << " [" << diagnostic.Code << "] "
                          << DiagnosticKindName(diagnostic.Kind) << ", count="
                          << diagnostic.Count;
                if (!diagnostic.Subject.empty())
                {
                    std::cout << ", subject=" << diagnostic.Subject;
                }
                std::cout << ": " << diagnostic.Message << "\n";

                if (diagnostic.Severity != GltfImportDiagnosticSeverityEXT::Warning)
                {
                    continue;
                }
                ++actionable;
                if (diagnosticOverlayLines_.size() >= 9)
                {
                    continue;
                }

                std::string prefix = "WARN ";
                if (diagnostic.Kind == GltfImportDiagnosticKindEXT::DroppedData ||
                    diagnostic.Kind == GltfImportDiagnosticKindEXT::UnsupportedFeature)
                {
                    prefix = "DROP ";
                }
                else if (diagnostic.Kind == GltfImportDiagnosticKindEXT::Approximation)
                {
                    prefix = "APPROX ";
                }
                std::string line = prefix + diagnostic.Code + " X" +
                                   std::to_string(diagnostic.Count);
                if (line.size() > 54)
                {
                    line.resize(51);
                    line += "...";
                }
                diagnosticOverlayLines_.push_back(std::move(line));
            }
        }

        diagnosticOverlayLines_.insert(
            diagnosticOverlayLines_.begin(),
            "GLTF IMPORT W" + std::to_string(warnings) + " / DROP " +
                std::to_string(dropped) + " / APPROX " + std::to_string(approximated));
        const std::size_t visibleActionable = diagnosticOverlayLines_.size() - 1;
        if (actionable == 0)
        {
            diagnosticOverlayLines_.push_back("CLEAN: NO FIDELITY WARNINGS");
        }
        else if (actionable > visibleActionable)
        {
            diagnosticOverlayLines_.push_back(
                "+" + std::to_string(actionable - visibleActionable) + " MORE WARNINGS");
        }

        getWindowProperty().setTitleProperty(
            "CNA glTF Viewer | W" + std::to_string(warnings) + " D" +
            std::to_string(dropped) + " A" + std::to_string(approximated));
    }

    void ViewerGame::Update(Microsoft::Xna::Framework::GameTime& gameTime)
    {
        UpdateAnimations(gameTime);

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
                pitch_ = MathHelper::Clamp(
                    pitch_ - static_cast<float>(deltaY) * 0.01f, -1.45f, 1.45f);
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

    void ViewerGame::UpdateAnimations(const Microsoft::Xna::Framework::GameTime& gameTime)
    {
        for (SkinPlayback& playback : skinPlaybacks_)
        {
            if (playback.player->getCurrentClipProperty() == nullptr)
            {
                continue;
            }
            playback.player->Update(gameTime.getElapsedGameTimeProperty(), true, true);
            ApplySkinPalette(*playback.skin, playback.player->GetSkinTransforms());
        }

        for (const RigidPlayback& playback : rigidPlaybacks_)
        {
            const double duration = playback.clip->Duration.getTotalSecondsProperty();
            const double total = gameTime.getTotalGameTimeProperty().getTotalSecondsProperty();
            const double position = duration > 0.0 ? std::fmod(total, duration) : 0.0;
            ApplyClipToBonesEXT(*playback.model, *playback.clip,
                                System::TimeSpan::FromSeconds(position));
        }
    }

    void ViewerGame::Draw(const Microsoft::Xna::Framework::GameTime& gameTime)
    {
        (void)gameTime;

        GraphicsDevice& device = getGraphicsDeviceProperty();
        device.Clear(Color(24, 29, 38, 255));

        const float horizontalDistance = std::cos(pitch_) * distance_;
        const Vector3 cameraPosition(
            target_.X + std::sin(yaw_) * horizontalDistance,
            target_.Y + std::sin(pitch_) * distance_,
            target_.Z + std::cos(yaw_) * horizontalDistance);
        const Matrix view = Matrix::CreateLookAt(cameraPosition, target_, Vector3::Up);

        const Viewport viewport = device.getViewportProperty();
        const float aspect = static_cast<float>(viewport.getWidthProperty()) /
                             static_cast<float>(std::max(viewport.getHeightProperty(), 1));
        const float nearPlane = std::max(sceneRadius_ * 0.001f, 0.001f);
        const float farPlane = std::max(distance_ + sceneRadius_ * 4.0f, 100.0f);
        const Matrix projection = Matrix::CreatePerspectiveFieldOfView(
            MathHelper::PiOver4, aspect, nearPlane, farPlane);

        DrawModels(view, projection, false);
        DrawModels(view, projection, true);
        DrawDiagnosticsOverlay();
        CaptureFirstFrame();
    }

    void ViewerGame::DrawModels(const Matrix& view, const Matrix& projection,
                                const bool transparentPass)
    {
        GraphicsDevice& device = getGraphicsDeviceProperty();
        const Matrix world = Matrix::getIdentityProperty();

        for (Model& model : models_)
        {
            std::vector<Matrix> absoluteBones(
                static_cast<std::size_t>(model.getBonesProperty().getCountProperty()));
            if (!absoluteBones.empty())
            {
                model.CopyAbsoluteBoneTransformsTo(absoluteBones);
            }

            for (ModelMesh* mesh : model.getMeshesProperty())
            {
                Matrix meshWorld = world;
                if (mesh->getParentBoneProperty() != nullptr && !absoluteBones.empty())
                {
                    const int index = mesh->getParentBoneProperty()->getIndexProperty();
                    meshWorld = absoluteBones.at(static_cast<std::size_t>(index)) * world;
                }
                const bool mirrored = meshWorld.Determinant() < 0.0f;

                for (ModelMeshPart* part : mesh->getMeshPartsProperty())
                {
                    Effect* effect = part != nullptr ? part->getEffectProperty() : nullptr;
                    if (effect == nullptr || part->getPrimitiveCountProperty() <= 0)
                    {
                        continue;
                    }

                    const bool transparent = AlphaMode(*effect) == AlphaModeEXT::Blend;
                    if (transparent != transparentPass)
                    {
                        continue;
                    }

                    auto* matrices = dynamic_cast<IEffectMatrices*>(effect);
                    if (matrices == nullptr)
                    {
                        throw std::runtime_error("Model effect does not implement IEffectMatrices.");
                    }
                    matrices->setWorldProperty(meshWorld);
                    matrices->setViewProperty(view);
                    matrices->setProjectionProperty(projection);

                    if (options_.noCull || IsDoubleSided(*effect))
                    {
                        device.setRasterizerStateProperty(RasterizerState::CullNone);
                    }
                    else
                    {
                        // glTF defines counter-clockwise triangles as front-facing, while XNA's
                        // CullCounterClockwise state removes that winding. Keep the authored
                        // front face by culling clockwise triangles, and reverse the decision for
                        // a placement whose world transform mirrors the geometry.
                        device.setRasterizerStateProperty(
                            mirrored ? RasterizerState::CullCounterClockwise
                                     : RasterizerState::CullClockwise);
                    }
                    device.setBlendStateProperty(
                        transparent ? BlendState::NonPremultiplied : BlendState::Opaque);
                    device.setDepthStencilStateProperty(
                        transparent ? DepthStencilState::DepthRead : DepthStencilState::Default);

                    const auto& samplers = part->getSamplerStatesEXTProperty();
                    for (std::size_t slot = 0; slot != samplers.size(); ++slot)
                    {
                        device.getSamplerStatesProperty()[static_cast<int>(slot)] = samplers[slot];
                    }

                    device.SetVertexBuffer(part->getVertexBufferProperty());
                    device.setIndicesProperty(part->getIndexBufferProperty());
                    EffectTechnique* technique = effect->getCurrentTechniqueProperty();
                    for (EffectPass& pass : technique->getPassesProperty())
                    {
                        pass.Apply();
                        device.DrawIndexedPrimitives(
                            part->getPrimitiveTypeEXTProperty(),
                            part->getVertexOffsetProperty(),
                            0,
                            part->getNumVerticesProperty(),
                            part->getStartIndexProperty(),
                            part->getPrimitiveCountProperty());
                    }
                }
            }
        }
    }

    void ViewerGame::DrawDiagnosticsOverlay()
    {
        if (!spriteBatch_ || !overlayPixel_ || diagnosticOverlayLines_.empty())
        {
            return;
        }

        std::size_t maximumCharacters = 0;
        for (const std::string& line : diagnosticOverlayLines_)
        {
            maximumCharacters = std::max(maximumCharacters, line.size());
        }
        const int panelWidth = static_cast<int>(maximumCharacters) * 12 + 24;
        const int panelHeight = static_cast<int>(diagnosticOverlayLines_.size()) * 20 + 20;

        spriteBatch_->Begin();
        spriteBatch_->Draw(*overlayPixel_, Rectangle(12, 12, panelWidth, panelHeight),
                           Color(0, 0, 0, 190));

        int y = 22;
        for (const std::string& line : diagnosticOverlayLines_)
        {
            Color color(225, 231, 239, 255);
            if (line.starts_with("DROP "))
            {
                color = Color(255, 105, 105, 255);
            }
            else if (line.starts_with("APPROX "))
            {
                color = Color(255, 205, 90, 255);
            }
            else if (line.starts_with("WARN "))
            {
                color = Color(255, 160, 90, 255);
            }
            else if (line.starts_with("CLEAN"))
            {
                color = Color(120, 230, 145, 255);
            }
            DrawPixelText(*spriteBatch_, *overlayPixel_, 24, y, line, color);
            y += 20;
        }
        spriteBatch_->End();
    }

    void ViewerGame::CaptureFirstFrame()
    {
        if (!options_.capturePath.has_value() || frameCaptured_)
        {
            return;
        }

        std::error_code error;
        const fs::path output = fs::absolute(*options_.capturePath, error);
        if (error)
        {
            throw std::runtime_error("Could not resolve --capture output: " + error.message());
        }
        if (!output.parent_path().empty())
        {
            fs::create_directories(output.parent_path(), error);
            if (error)
            {
                throw std::runtime_error("Could not create --capture directory: " + error.message());
            }
        }

        GraphicsDevice& device = getGraphicsDeviceProperty();
        const Viewport viewport = device.getViewportProperty();
        const int width = viewport.getWidthProperty();
        const int height = viewport.getHeightProperty();
        std::vector<Color> pixels(
            static_cast<std::size_t>(width) * static_cast<std::size_t>(height),
            Color::Transparent);
        const Rectangle region(0, 0, width, height);
        device.GetBackBufferData(&region, pixels.data(), 0, static_cast<int>(pixels.size()));

        std::vector<std::uint8_t> rgba(pixels.size() * 4);
        for (std::size_t index = 0; index != pixels.size(); ++index)
        {
            rgba[index * 4 + 0] = pixels[index].getRProperty();
            rgba[index * 4 + 1] = pixels[index].getGProperty();
            rgba[index * 4 + 2] = pixels[index].getBProperty();
            rgba[index * 4 + 3] = pixels[index].getAProperty();
        }
        Texture2D capture = Texture2D::CreateFromPixels(device, width, height, rgba);
        capture.SaveAsPng(output.string());
        frameCaptured_ = true;
        std::cout << "Captured first rendered frame to " << output << ".\n";
        Exit();
    }

    void ViewerGame::ResetCamera()
    {
        yaw_ = 0.7f;
        pitch_ = 0.35f;
        // At a 45-degree vertical field of view, 2.4 radii only just reaches the viewport edges.
        // Leave a deliberate margin so conservative spheres and antialiased edge pixels remain
        // visible instead of making a correctly framed asset look clipped.
        distance_ = std::max(sceneRadius_ * 3.0f, 1.5f);
    }

    void ViewerGame::ConfigureCameraFromModels()
    {
        std::optional<BoundingSphere> bounds;
        for (const Model& model : models_)
        {
            const std::optional<BoundingSphere> modelBounds =
                model.getBoundingSphereEXTProperty();
            if (!modelBounds.has_value())
            {
                continue;
            }
            bounds = bounds.has_value()
                ? BoundingSphere::CreateMerged(*bounds, *modelBounds)
                : modelBounds;
        }

        if (bounds.has_value() && std::isfinite(bounds->Radius) && bounds->Radius >= 0.0f)
        {
            target_ = bounds->Center;
            sceneRadius_ = std::max(bounds->Radius, 0.1f);
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
