#pragma once

#include "CnjConversion.hpp"
#include "CommandLine.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Input/MouseState.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"

namespace CnaGltfViewer
{
    class ViewerGame final : public Microsoft::Xna::Framework::Game
    {
    public:
        explicit ViewerGame(ViewerOptions options);
        ~ViewerGame() override;

        GetTypeNameHPP()

    protected:
        void LoadContent() override;
        void Update(Microsoft::Xna::Framework::GameTime& gameTime) override;
        void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

    private:
        struct SkinPlayback
        {
            const Microsoft::Xna::Framework::Graphics::ModelSkinEXT* skin = nullptr;
            std::unique_ptr<Microsoft::Xna::Framework::Graphics::AnimationPlayer> player;
        };

        struct RigidPlayback
        {
            Microsoft::Xna::Framework::Graphics::Model* model = nullptr;
            const Microsoft::Xna::Framework::Graphics::AnimationClip* clip = nullptr;
        };

        struct ImportedCameraSelection
        {
            Microsoft::Xna::Framework::Graphics::Model* model = nullptr;
            std::size_t cameraIndex = 0;
        };

        void LoadModels();
        void ConfigureCameraFromModels();
        void ConfigureFallbackLighting();
        void ConfigureAnimations();
        void ConfigureDiagnostics();
        void UpdateAnimations(const Microsoft::Xna::Framework::GameTime& gameTime);
        void DrawModels(const Microsoft::Xna::Framework::Matrix& view,
                        const Microsoft::Xna::Framework::Matrix& projection,
                        bool transparentPass);
        void DrawDiagnosticsOverlay();
        void CaptureFirstFrame();
        void ResetCamera();

        ViewerOptions options_;
        Microsoft::Xna::Framework::GraphicsDeviceManager graphics_;
        std::optional<ConvertedScene> convertedScene_;
        std::vector<Microsoft::Xna::Framework::Graphics::Model> models_;
        std::vector<SkinPlayback> skinPlaybacks_;
        std::vector<RigidPlayback> rigidPlaybacks_;
        std::optional<ImportedCameraSelection> importedCamera_;
        std::vector<std::string> diagnosticOverlayLines_;
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::SpriteBatch> spriteBatch_;
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::Texture2D> overlayPixel_;
        std::optional<Microsoft::Xna::Framework::Input::MouseState> previousMouse_;
        Microsoft::Xna::Framework::Vector3 target_;
        float sceneRadius_ = 1.0f;
        float yaw_ = 0.7f;
        float pitch_ = 0.35f;
        float distance_ = 3.0f;
        bool frameCaptured_ = false;
    };
}
