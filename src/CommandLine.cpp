#include "CommandLine.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace CnaGltfViewer
{
    namespace
    {
        [[nodiscard]] float ParsePositiveScale(const std::string_view value)
        {
            std::size_t parsedCharacters = 0;
            float scale = 0.0f;

            try
            {
                scale = std::stof(std::string(value), &parsedCharacters);
            }
            catch (const std::exception&)
            {
                throw std::invalid_argument("--scale must be a positive number.");
            }

            if (parsedCharacters != value.size() || !std::isfinite(scale) || scale <= 0.0f)
            {
                throw std::invalid_argument("--scale must be a positive number.");
            }

            return scale;
        }

        [[nodiscard]] bool IsGltfPath(const std::filesystem::path& path)
        {
            const std::string extension = path.extension().string();
            return extension == ".gltf" || extension == ".glb";
        }
    }

    CommandLineResult ParseCommandLine(const int argc, char* argv[])
    {
        if (argc == 1)
        {
            return {.options = std::nullopt, .showHelp = true};
        }

        CommandLineResult result;
        ViewerOptions options;
        bool hasInputPath = false;

        for (int index = 1; index < argc; ++index)
        {
            const std::string_view argument(argv[index]);
            if (argument == "--help" || argument == "-h")
            {
                if (argc != 2)
                {
                    throw std::invalid_argument("--help cannot be combined with other arguments.");
                }
                return {.options = std::nullopt, .showHelp = true};
            }

            if (argument == "--scale")
            {
                if (++index >= argc)
                {
                    throw std::invalid_argument("--scale requires a value.");
                }
                options.unitScale = ParsePositiveScale(argv[index]);
                continue;
            }

            if (argument == "--output")
            {
                if (++index >= argc)
                {
                    throw std::invalid_argument("--output requires an empty directory path.");
                }
                if (options.outputDirectory.has_value())
                {
                    throw std::invalid_argument("--output may be specified only once.");
                }
                options.outputDirectory = std::filesystem::path(argv[index]);
                continue;
            }

            if (argument.starts_with('-'))
            {
                throw std::invalid_argument("Unknown option: " + std::string(argument));
            }

            if (hasInputPath)
            {
                throw std::invalid_argument("Only one glTF input path may be supplied.");
            }

            options.inputPath = std::filesystem::path(argument);
            hasInputPath = true;
        }

        if (!hasInputPath)
        {
            throw std::invalid_argument("A .gltf or .glb input path is required.");
        }
        if (!IsGltfPath(options.inputPath))
        {
            throw std::invalid_argument("The input file must have a .gltf or .glb extension.");
        }

        result.options = std::move(options);
        return result;
    }

    void PrintUsage(const char* executableName)
    {
        std::cout
            << "Usage: " << executableName
            << " <model.gltf|model.glb> [--scale <positive-number>] [--output <empty-directory>]\n\n"
            << "Converts glTF to CNA CNJ and displays the generated model.\n\n"
            << "Controls:\n"
            << "  Left mouse drag  Orbit camera\n"
            << "  Mouse wheel      Zoom\n"
            << "  R                Reset camera\n"
            << "  Esc              Exit\n";
    }
}
