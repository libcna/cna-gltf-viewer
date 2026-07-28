#pragma once

#include <filesystem>
#include <optional>

namespace CnaGltfViewer
{
    struct ViewerOptions
    {
        std::filesystem::path inputPath;
        float unitScale = 1.0f;
        std::optional<std::filesystem::path> outputDirectory;
    };

    struct CommandLineResult
    {
        std::optional<ViewerOptions> options;
        bool showHelp = false;
    };

    CommandLineResult ParseCommandLine(int argc, char* argv[]);
    void PrintUsage(const char* executableName);
}
