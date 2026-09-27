#include "ViewerGame.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
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
    using Microsoft::Xna::Framework::Input::KeyboardState;
    using Microsoft::Xna::Framework::Input::Keys;

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

        [[nodiscard]] bool IsGltfFile(const fs::path& path)
        {
            std::string extension = path.extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(),
                           [](const unsigned char character)
                           {
                               return static_cast<char>(std::tolower(character));
                           });
            return extension == ".gltf" || extension == ".glb";
        }

        [[nodiscard]] std::string TruncateText(const std::string& text,
                                                const std::size_t maximumCharacters)
        {
            if (text.size() <= maximumCharacters)
            {
                return text;
            }
            if (maximumCharacters <= 3)
            {
                return text.substr(0, maximumCharacters);
            }
            return text.substr(0, maximumCharacters - 3) + "...";
        }

        [[nodiscard]] std::string DisplayPath(const fs::path& path,
                                               const std::size_t maximumCharacters)
        {
            std::string value = path.generic_string();
            if (value.empty())
            {
                value = ".";
            }
            return TruncateText(value, maximumCharacters);
        }

        [[nodiscard]] std::string DisplayEntryLabel(const fs::path& path,
                                                     const bool parentDirectory,
                                                     const bool directory,
                                                     const std::size_t maximumCharacters)
        {
            if (parentDirectory)
            {
                return "..";
            }

            const std::string prefix = directory
                ? "DIR "
                : "      ";
            return TruncateText(prefix + path.filename().string(), maximumCharacters);
        }
    }

    ViewerGame::ViewerGame(ViewerOptions options)
        : options_(std::move(options)),
          graphics_(this)
    {
        getWindowProperty().setTitleProperty("CNA glTF Viewer");
        if (options_.capturePath.has_value())
        {
            graphics_.setGraphicsProfileProperty(
                Microsoft::Xna::Framework::Graphics::GraphicsProfile::HiDef);
        }
        // Match CNA's initial desktop window size. This also keeps an Xvfb capture exact: a bare
        // X11 server has no window manager to acknowledge a later asynchronous resize request.
        graphics_.setPreferredBackBufferWidthProperty(options_.referenceCapture ? 512 : 800);
        graphics_.setPreferredBackBufferHeightProperty(options_.referenceCapture ? 512 : 480);
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
        InitializeOverlay();
        if (options_.inputPath.empty())
        {
            std::error_code error;
            const fs::path directory = fs::current_path(error);
            if (error)
            {
                throw std::runtime_error("Could not determine the current directory: " +
                                         error.message());
            }
            OpenFileBrowser(directory);
            return;
        }

        LoadScene(options_.inputPath, false);
    }

    void ViewerGame::InitializeOverlay()
    {
        GraphicsDevice& device = getGraphicsDeviceProperty();
        spriteBatch_ = std::make_unique<SpriteBatch>(device);
        overlayPixel_ = std::make_unique<Texture2D>(device, 1, 1);
        const Color white = Color::White;
        overlayPixel_->SetData(&white, 1);
    }

    void ViewerGame::LoadScene(const fs::path& inputPath, const bool fromFileBrowser)
    {
        ViewerOptions loadOptions = options_;
        loadOptions.inputPath = inputPath;
        if (fromFileBrowser)
        {
            // Command-line output/capture destinations describe one startup load. A model
            // selected later must be able to load repeatedly, so give it a fresh temporary CNJ
            // directory and do not reuse one-shot capture/oracle settings. Scene-specific
            // selectors are also cleared because another file need not contain the same clip or
            // camera.
            loadOptions.outputDirectory.reset();
            loadOptions.oracleOutputDirectory.reset();
            loadOptions.capturePath.reset();
            loadOptions.referenceCapture = false;
            loadOptions.clipName.reset();
            loadOptions.animationTimeSeconds.reset();
            loadOptions.cameraSelector.reset();
        }

        ClearLoadedScene();
        options_ = std::move(loadOptions);
        LoadModels(options_);
        ConfigureFallbackLighting();
        ConfigureAnimations();
        ConfigureDiagnostics();
        ConfigureCameraFromModels();
        frameCaptured_ = false;
    }

    void ViewerGame::ClearLoadedScene()
    {
        // Playback objects contain pointers into models_, so they must be destroyed first.
        skinPlaybacks_.clear();
        rigidPlaybacks_.clear();
        importedCamera_.reset();
        diagnosticOverlayLines_.clear();
        models_.clear();
        getContentProperty().Unload();

        if (convertedScene_.has_value() && convertedScene_->temporaryOutput)
        {
            std::error_code error;
            fs::remove_all(convertedScene_->outputDirectory, error);
            if (error)
            {
                std::cerr << "warning: could not remove temporary CNJ directory "
                          << convertedScene_->outputDirectory << ": " << error.message() << "\n";
            }
        }
        convertedScene_.reset();
    }

    void ViewerGame::LoadModels(const ViewerOptions& loadOptions)
    {
        // This diagnostic is deliberately available on both load paths. It invokes the exact
        // converter embedded with this viewer, so a direct-render investigation and an offline
        // CNJ investigation can emit the same deterministic L2-L5 evidence before rendering.
        CnjConverter::DumpOracle(loadOptions);

        std::error_code error;
        const fs::path input = fs::absolute(loadOptions.inputPath, error);
        if (error || !fs::is_regular_file(input, error))
        {
            throw std::runtime_error(
                "Input glTF file does not exist or is not a regular file: " +
                loadOptions.inputPath.string());
        }

        if (loadOptions.direct)
        {
            getContentProperty().setRootDirectoryProperty(input.parent_path().string());
            models_.reserve(1);
            models_.push_back(getContentProperty().Load<Model>(input.filename().string()));
            std::cout << "Loaded glTF directly through ContentManager: " << input << "\n";
            return;
        }

        convertedScene_ = CnjConverter::Convert(loadOptions);
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
        const System::TimeSpan initialTime = options_.animationTimeSeconds.has_value()
            ? System::TimeSpan::FromSeconds(*options_.animationTimeSeconds)
            : System::TimeSpan::Zero;

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
                playback.player->Update(initialTime, false, true);
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
                        const double duration = clip.Duration.getTotalSecondsProperty();
                        const double requested = initialTime.getTotalSecondsProperty();
                        const double position =
                            duration > 0.0 ? std::fmod(requested, duration) : 0.0;
                        ApplyClipToBonesEXT(
                            model, clip, System::TimeSpan::FromSeconds(position));
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

        if (options_.animationTimeSeconds.has_value())
        {
            std::cout << "Showing animation clip '" << *options_.clipName
                      << "' at fixed looping time " << *options_.animationTimeSeconds
                      << " second(s).\n";
        }
        else if (options_.clipName.has_value())
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
        if (fileBrowserActive_)
        {
            HandleFileBrowserInput(
                keyboard, gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
            previousKeyboard_ = keyboard;
            return;
        }

        if (keyboard.IsKeyDown(Keys::Escape))
        {
            Exit();
            return;
        }
        if (keyboard.IsKeyDown(Keys::R))
        {
            ResetCamera();
        }
        if (IsKeyPressed(keyboard, Keys::O))
        {
            std::error_code error;
            fs::path directory = options_.inputPath.empty()
                ? fs::current_path(error)
                : fs::absolute(options_.inputPath, error).parent_path();
            if (error)
            {
                fileBrowserStatus_ = "ERROR: " + error.message();
            }
            else
            {
                OpenFileBrowser(directory);
            }
            previousKeyboard_ = keyboard;
            return;
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
        previousKeyboard_ = keyboard;
    }

    bool ViewerGame::IsKeyPressed(
        const Microsoft::Xna::Framework::Input::KeyboardState& keyboard,
        const Microsoft::Xna::Framework::Input::Keys key) const
    {
        return keyboard.IsKeyDown(key) &&
               (!previousKeyboard_.has_value() || previousKeyboard_->IsKeyUp(key));
    }

    void ViewerGame::OpenFileBrowser(const fs::path& directory)
    {
        std::error_code error;
        fs::path absoluteDirectory = fs::absolute(directory, error);
        if (error)
        {
            fileBrowserStatus_ = "ERROR: " + error.message();
            fileBrowserActive_ = true;
            fileBrowserEntries_.clear();
            return;
        }
        absoluteDirectory = absoluteDirectory.lexically_normal();
        if (!fs::is_directory(absoluteDirectory, error) || error)
        {
            fileBrowserStatus_ = "ERROR: not a directory";
            fileBrowserActive_ = true;
            fileBrowserEntries_.clear();
            return;
        }

        fileBrowserDirectory_ = std::move(absoluteDirectory);
        fileBrowserActive_ = true;
        fileBrowserSelection_ = 0;
        fileBrowserFirstVisible_ = 0;
        RefreshFileBrowser();
    }

    void ViewerGame::RefreshFileBrowser()
    {
        fileBrowserEntries_.clear();
        fileBrowserSelection_ = 0;
        fileBrowserFirstVisible_ = 0;

        std::error_code error;
        const fs::path parent = fileBrowserDirectory_.parent_path();
        if (!parent.empty() && parent != fileBrowserDirectory_)
        {
            fileBrowserEntries_.push_back({
                .kind = FileBrowserEntry::Kind::ParentDirectory,
                .path = parent,
                .label = ".."});
        }

        std::vector<fs::path> directories;
        std::vector<fs::path> models;
        fs::directory_iterator iterator(fileBrowserDirectory_, error);
        if (error)
        {
            fileBrowserStatus_ = "ERROR: cannot read directory";
            return;
        }

        const fs::directory_iterator end;
        for (; iterator != end; iterator.increment(error))
        {
            if (error)
            {
                fileBrowserStatus_ = "ERROR: cannot read directory";
                return;
            }
            const fs::directory_entry& entry = *iterator;
            std::error_code entryError;
            if (entry.is_directory(entryError))
            {
                directories.push_back(entry.path());
            }
            else if (!entryError && entry.is_regular_file(entryError) && IsGltfFile(entry.path()))
            {
                models.push_back(entry.path());
            }
        }

        const auto pathLess = [](const fs::path& left, const fs::path& right)
        {
            std::string leftName = left.filename().string();
            std::string rightName = right.filename().string();
            std::transform(leftName.begin(), leftName.end(), leftName.begin(),
                           [](const unsigned char character)
                           {
                               return static_cast<char>(std::tolower(character));
                           });
            std::transform(rightName.begin(), rightName.end(), rightName.begin(),
                           [](const unsigned char character)
                           {
                               return static_cast<char>(std::tolower(character));
                           });
            return leftName == rightName ? left < right : leftName < rightName;
        };
        std::sort(directories.begin(), directories.end(), pathLess);
        std::sort(models.begin(), models.end(), pathLess);

        constexpr std::size_t maximumLabelCharacters = 56;
        for (const fs::path& path : directories)
        {
            fileBrowserEntries_.push_back({
                .kind = FileBrowserEntry::Kind::Directory,
                .path = path,
                .label = DisplayEntryLabel(path, false, true, maximumLabelCharacters)});
        }
        for (const fs::path& path : models)
        {
            fileBrowserEntries_.push_back({
                .kind = FileBrowserEntry::Kind::ModelFile,
                .path = path,
                .label = DisplayEntryLabel(path, false, false, maximumLabelCharacters)});
        }
        fileBrowserStatus_.clear();
    }

    void ViewerGame::KeepFileBrowserSelectionVisible()
    {
        constexpr std::size_t visibleEntries = 17;
        if (fileBrowserSelection_ < fileBrowserFirstVisible_)
        {
            fileBrowserFirstVisible_ = fileBrowserSelection_;
        }
        else if (fileBrowserSelection_ >= fileBrowserFirstVisible_ + visibleEntries)
        {
            fileBrowserFirstVisible_ = fileBrowserSelection_ - visibleEntries + 1;
        }
    }

    void ViewerGame::HandleFileBrowserInput(
        const Microsoft::Xna::Framework::Input::KeyboardState& keyboard,
        const double elapsedSeconds)
    {
        if (IsKeyPressed(keyboard, Keys::Escape))
        {
            if (models_.empty())
            {
                Exit();
            }
            else
            {
                fileBrowserActive_ = false;
                fileBrowserStatus_.clear();
            }
            return;
        }

        const bool up = keyboard.IsKeyDown(Keys::Up);
        const bool down = keyboard.IsKeyDown(Keys::Down);
        if (up != down)
        {
            const Keys arrow = up ? Keys::Up : Keys::Down;
            if (!fileBrowserHeldArrow_.has_value() || *fileBrowserHeldArrow_ != arrow)
            {
                fileBrowserHeldArrow_ = arrow;
                fileBrowserArrowHeldSeconds_ = 0.0;
                fileBrowserArrowRepeatSeconds_ = 0.0;
                fileBrowserArrowRepeating_ = false;
                MoveFileBrowserSelection(up ? -1 : 1);
            }
            else if (!fileBrowserEntries_.empty())
            {
                constexpr double repeatDelay = 0.30;
                constexpr double repeatInterval = 0.075;
                const double safeElapsedSeconds = std::max(elapsedSeconds, 0.0);
                if (!fileBrowserArrowRepeating_)
                {
                    fileBrowserArrowHeldSeconds_ += safeElapsedSeconds;
                    if (fileBrowserArrowHeldSeconds_ >= repeatDelay)
                    {
                        fileBrowserArrowRepeating_ = true;
                        fileBrowserArrowRepeatSeconds_ = 0.0;
                        MoveFileBrowserSelection(up ? -1 : 1);
                    }
                }
                else
                {
                    fileBrowserArrowRepeatSeconds_ += safeElapsedSeconds;
                    while (fileBrowserArrowRepeatSeconds_ >= repeatInterval)
                    {
                        fileBrowserArrowRepeatSeconds_ -= repeatInterval;
                        MoveFileBrowserSelection(up ? -1 : 1);
                    }
                }
            }
        }
        else
        {
            fileBrowserHeldArrow_.reset();
            fileBrowserArrowHeldSeconds_ = 0.0;
            fileBrowserArrowRepeatSeconds_ = 0.0;
            fileBrowserArrowRepeating_ = false;
        }
        if (IsKeyPressed(keyboard, Keys::PageUp) && !fileBrowserEntries_.empty())
        {
            constexpr std::size_t pageSize = 17;
            fileBrowserSelection_ = fileBrowserSelection_ > pageSize
                ? fileBrowserSelection_ - pageSize
                : 0;
            KeepFileBrowserSelectionVisible();
        }
        if (IsKeyPressed(keyboard, Keys::PageDown) && !fileBrowserEntries_.empty())
        {
            constexpr std::size_t pageSize = 17;
            fileBrowserSelection_ = std::min(
                fileBrowserSelection_ + pageSize, fileBrowserEntries_.size() - 1);
            KeepFileBrowserSelectionVisible();
        }
        if (IsKeyPressed(keyboard, Keys::Home) && !fileBrowserEntries_.empty())
        {
            fileBrowserSelection_ = 0;
            KeepFileBrowserSelectionVisible();
        }
        if (IsKeyPressed(keyboard, Keys::End) && !fileBrowserEntries_.empty())
        {
            fileBrowserSelection_ = fileBrowserEntries_.size() - 1;
            KeepFileBrowserSelectionVisible();
        }
        if (IsKeyPressed(keyboard, Keys::Back))
        {
            const fs::path parent = fileBrowserDirectory_.parent_path();
            if (!parent.empty() && parent != fileBrowserDirectory_)
            {
                OpenFileBrowser(parent);
            }
            return;
        }
        if (IsKeyPressed(keyboard, Keys::Enter) && !fileBrowserEntries_.empty())
        {
            SelectFileBrowserEntry();
        }
    }

    void ViewerGame::MoveFileBrowserSelection(const int direction)
    {
        if (fileBrowserEntries_.empty())
        {
            return;
        }
        if (direction < 0)
        {
            fileBrowserSelection_ = fileBrowserSelection_ == 0
                ? fileBrowserEntries_.size() - 1
                : fileBrowserSelection_ - 1;
        }
        else if (direction > 0)
        {
            fileBrowserSelection_ = (fileBrowserSelection_ + 1) % fileBrowserEntries_.size();
        }
        KeepFileBrowserSelectionVisible();
    }

    void ViewerGame::SelectFileBrowserEntry()
    {
        const FileBrowserEntry entry = fileBrowserEntries_.at(fileBrowserSelection_);
        if (entry.kind == FileBrowserEntry::Kind::ParentDirectory ||
            entry.kind == FileBrowserEntry::Kind::Directory)
        {
            OpenFileBrowser(entry.path);
            return;
        }

        try
        {
            LoadScene(entry.path, true);
            fileBrowserActive_ = false;
            fileBrowserStatus_.clear();
            std::cout << "Opened glTF file from the file browser: " << entry.path << "\n";
        }
        catch (const std::exception& error)
        {
            ClearLoadedScene();
            fileBrowserActive_ = true;
            fileBrowserStatus_ = std::string("ERROR: ") + error.what();
            std::cerr << "cna-gltf-viewer: " << error.what() << '\n';
        }
    }

    void ViewerGame::UpdateAnimations(const Microsoft::Xna::Framework::GameTime& gameTime)
    {
        if (options_.animationTimeSeconds.has_value())
        {
            return;
        }
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
        device.Clear(options_.referenceCapture ? Color::Transparent : Color(24, 29, 38, 255));

        const Viewport viewport = device.getViewportProperty();
        const float aspect = static_cast<float>(viewport.getWidthProperty()) /
                             static_cast<float>(std::max(viewport.getHeightProperty(), 1));

        Matrix view;
        Matrix projection;
        if (importedCamera_.has_value())
        {
            Model& model = *importedCamera_->model;
            const ModelCameraEXT& camera =
                model.getCamerasEXTProperty().at(importedCamera_->cameraIndex);
            Matrix cameraWorld = camera.WorldTransform;
            if (camera.SceneNodeIndex >= 0)
            {
                std::vector<Matrix> absoluteBones(
                    static_cast<std::size_t>(model.getBonesProperty().getCountProperty()));
                if (static_cast<std::size_t>(camera.SceneNodeIndex) >= absoluteBones.size())
                {
                    throw std::runtime_error(
                        "Selected imported camera refers to a scene node outside Model::Bones.");
                }
                model.CopyAbsoluteBoneTransformsTo(absoluteBones);
                cameraWorld = absoluteBones[static_cast<std::size_t>(camera.SceneNodeIndex)];
            }
            const float determinant = cameraWorld.Determinant();
            if (!std::isfinite(determinant) || std::abs(determinant) < 1e-8f)
            {
                throw std::runtime_error(
                    "Selected imported camera has a non-invertible world transform.");
            }
            view = Matrix::Invert(cameraWorld);
            projection = camera.Projection;
            if (camera.IsPerspective && !camera.HasAuthoredAspectRatio)
            {
                projection = camera.HasInfiniteFarPlane
                    ? CreateInfinitePerspectiveFieldOfViewEXT(
                          camera.FieldOfView, aspect, camera.NearPlaneDistance)
                    : Matrix::CreatePerspectiveFieldOfView(
                          camera.FieldOfView, aspect, camera.NearPlaneDistance,
                          camera.FarPlaneDistance);
            }
        }
        else
        {
            const float horizontalDistance = std::cos(pitch_) * distance_;
            const Vector3 cameraPosition(
                target_.X + std::sin(yaw_) * horizontalDistance,
                target_.Y + std::sin(pitch_) * distance_,
                target_.Z + std::cos(yaw_) * horizontalDistance);
            view = Matrix::CreateLookAt(cameraPosition, target_, Vector3::Up);

            const float nearPlane = std::max(sceneRadius_ * 0.001f, 0.001f);
            const float farPlane = std::max(distance_ + sceneRadius_ * 4.0f, 100.0f);
            projection = Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, aspect, nearPlane, farPlane);
        }

        DrawModels(view, projection, false);
        DrawModels(view, projection, true);
        if (!options_.referenceCapture)
        {
            DrawDiagnosticsOverlay();
            if (fileBrowserActive_)
            {
                DrawFileBrowserOverlay();
            }
        }
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

    void ViewerGame::DrawFileBrowserOverlay()
    {
        if (!spriteBatch_ || !overlayPixel_)
        {
            return;
        }

        const Viewport viewport = getGraphicsDeviceProperty().getViewportProperty();
        const int width = viewport.getWidthProperty();
        const int height = viewport.getHeightProperty();
        const int panelWidth = std::max(width - 24, 240);
        const int panelHeight = std::max(height - 24, 180);
        const std::size_t maximumCharacters = static_cast<std::size_t>(
            std::max((panelWidth - 48) / 12, 20));
        constexpr std::size_t visibleEntries = 17;

        spriteBatch_->Begin();
        spriteBatch_->Draw(*overlayPixel_, Rectangle(12, 12, panelWidth, panelHeight),
                           Color(0, 0, 0, 238));

        DrawPixelText(*spriteBatch_, *overlayPixel_, 24, 22, "OPEN GLTF OR GLB", Color::White);
        DrawPixelText(*spriteBatch_, *overlayPixel_, 24, 42,
                      "DIR " + DisplayPath(fileBrowserDirectory_, maximumCharacters),
                      Color(180, 205, 235, 255));
        DrawPixelText(*spriteBatch_, *overlayPixel_, 24, 62,
                      "UP DOWN PAGE HOME END MOVE ENTER OPEN BACK ESC CLOSE",
                      Color(210, 220, 235, 255));

        if (fileBrowserEntries_.empty())
        {
            DrawPixelText(*spriteBatch_, *overlayPixel_, 24, 100,
                          "NO GLTF OR GLB FILES IN THIS DIRECTORY",
                          Color(235, 235, 235, 255));
        }
        else
        {
            const std::size_t end = std::min(
                fileBrowserEntries_.size(), fileBrowserFirstVisible_ + visibleEntries);
            int y = 100;
            for (std::size_t index = fileBrowserFirstVisible_; index < end; ++index)
            {
                const bool selected = index == fileBrowserSelection_;
                const std::string line = std::string(selected ? "> " : "  ") +
                                         fileBrowserEntries_[index].label;
                const Color color = selected ? Color(255, 235, 125, 255)
                                             : Color(235, 235, 235, 255);
                DrawPixelText(*spriteBatch_, *overlayPixel_, 24, y,
                              TruncateText(line, maximumCharacters), color);
                y += 20;
            }
        }

        if (!fileBrowserStatus_.empty())
        {
            DrawPixelText(*spriteBatch_, *overlayPixel_, 24, height - 42,
                          TruncateText(fileBrowserStatus_, maximumCharacters),
                          Color(255, 135, 115, 255));
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
        const auto& presentation = device.getPresentationParametersProperty();
        const int width = presentation.getBackBufferWidthProperty();
        const int height = presentation.getBackBufferHeightProperty();
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

        if (options_.referenceCapture && !options_.cameraSelector.has_value())
        {
            const float nearPlane = std::max(sceneRadius_ * 0.001f, 0.001f);
            const float farPlane = std::max(distance_ + sceneRadius_ * 4.0f, 100.0f);
            std::ostringstream metadata;
            metadata << std::setprecision(std::numeric_limits<float>::max_digits10)
                     << "CNA_REFERENCE_CAMERA={\"target\":[" << target_.X << ','
                     << target_.Y << ',' << target_.Z << "],\"sceneRadius\":"
                     << sceneRadius_ << ",\"distance\":" << distance_
                     << ",\"near\":" << nearPlane << ",\"far\":" << farPlane
                     << ",\"yaw\":" << yaw_ << ",\"pitch\":" << pitch_ << '}';
            std::cout << metadata.str() << '\n';
        }

        if (!options_.cameraSelector.has_value())
        {
            // GLTF-323: imported cameras never silently replace the viewer's orbit/framing camera.
            return;
        }

        std::vector<ImportedCameraSelection> cameras;
        for (Model& model : models_)
        {
            for (std::size_t index = 0; index < model.getCamerasEXTProperty().size(); ++index)
            {
                cameras.push_back({&model, index});
            }
        }
        if (cameras.empty())
        {
            throw std::runtime_error("--camera was requested, but the glTF scene has no camera.");
        }

        const std::string& selector = *options_.cameraSelector;
        if (selector.starts_with('#'))
        {
            std::size_t index = 0;
            const char* begin = selector.data() + 1;
            const char* end = selector.data() + selector.size();
            const auto [parsed, error] = std::from_chars(begin, end, index);
            if (begin == end || error != std::errc{} || parsed != end || index >= cameras.size())
            {
                throw std::runtime_error(
                    "Imported camera selector '" + selector + "' is not a valid available #index.");
            }
            importedCamera_ = cameras[index];
        }
        else
        {
            for (const ImportedCameraSelection& candidate : cameras)
            {
                const ModelCameraEXT& camera =
                    candidate.model->getCamerasEXTProperty().at(candidate.cameraIndex);
                if (camera.Name != selector)
                {
                    continue;
                }
                if (importedCamera_.has_value())
                {
                    throw std::runtime_error(
                        "Imported camera name '" + selector +
                        "' is ambiguous; select it by global #index instead.");
                }
                importedCamera_ = candidate;
            }
            if (!importedCamera_.has_value())
            {
                throw std::runtime_error(
                    "No imported camera named '" + selector + "' exists in the loaded scene.");
            }
        }

        const ModelCameraEXT& selected = importedCamera_->model->getCamerasEXTProperty().at(
            importedCamera_->cameraIndex);
        std::cout << "Using imported camera '"
                  << (selected.Name.empty() ? "<unnamed>" : selected.Name)
                  << "' by explicit --camera request; the viewer orbit camera remains the default.\n";
    }

    GetTypeNameCPP(ViewerGame, "CnaGltfViewer.ViewerGame")
}
