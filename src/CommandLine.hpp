#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace CnaGltfViewer
{
    struct ViewerOptions
    {
        std::filesystem::path inputPath;
        float unitScale = 1.0f;
        std::optional<std::filesystem::path> outputDirectory;
        std::optional<std::filesystem::path> oracleOutputDirectory;
        std::optional<std::filesystem::path> capturePath;
        std::optional<std::string> clipName;
        std::optional<double> animationTimeSeconds;
        std::optional<std::string> cameraSelector;
        bool direct = false;
        bool noCull = false;
        bool referenceCapture = false;
    };

    struct CommandLineResult
    {
        std::optional<ViewerOptions> options;
        bool showHelp = false;
    };

    CommandLineResult ParseCommandLine(int argc, char* argv[]);
    void PrintUsage(const char* executableName);
}
