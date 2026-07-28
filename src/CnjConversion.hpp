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
    };

    class CnjConverter
    {
    public:
        [[nodiscard]] static ConvertedScene Convert(const ViewerOptions& options);
    };
}
