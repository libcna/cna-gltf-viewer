#pragma once

#include "CommandLine.hpp"

#include <filesystem>
#include <vector>

namespace CnaGltfViewer
{
    struct ConvertedScene
    {
        std::filesystem::path outputDirectory;
        std::vector<std::filesystem::path> modelAssets;
        bool temporaryOutput = false;
    };

    class CnjConverter
    {
    public:
        [[nodiscard]] static ConvertedScene Convert(const ViewerOptions& options);
        static void DumpOracle(const ViewerOptions& options);
    };
}
