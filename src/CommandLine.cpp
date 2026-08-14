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

        [[nodiscard]] std::filesystem::path NormalizeOutputPath(
            const std::filesystem::path& path)
        {
            std::error_code error;
            std::filesystem::path normalized = std::filesystem::weakly_canonical(path, error);
            if (!error)
            {
                return normalized;
            }

            error.clear();
            normalized = std::filesystem::absolute(path, error);
            return (error ? path : normalized).lexically_normal();
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

            if (argument == "--dump-oracle")
            {
                if (++index >= argc || std::string_view(argv[index]).empty())
                {
                    throw std::invalid_argument(
                        "--dump-oracle requires an empty output directory path.");
                }
                if (options.oracleOutputDirectory.has_value())
                {
                    throw std::invalid_argument("--dump-oracle may be specified only once.");
                }
                options.oracleOutputDirectory = std::filesystem::path(argv[index]);
                continue;
            }

            if (argument == "--capture")
            {
                if (++index >= argc)
                {
                    throw std::invalid_argument("--capture requires a .png output path.");
                }
                if (options.capturePath.has_value())
                {
                    throw std::invalid_argument("--capture may be specified only once.");
                }
                options.capturePath = std::filesystem::path(argv[index]);
                if (options.capturePath->extension() != ".png")
                {
                    throw std::invalid_argument("--capture output must have a .png extension.");
                }
                continue;
            }

            if (argument == "--clip")
            {
                if (++index >= argc || std::string_view(argv[index]).empty())
                {
                    throw std::invalid_argument("--clip requires a non-empty clip name.");
                }
                if (options.clipName.has_value())
                {
                    throw std::invalid_argument("--clip may be specified only once.");
                }
                options.clipName = std::string(argv[index]);
                continue;
            }

            if (argument == "--direct")
            {
                options.direct = true;
                continue;
            }

            if (argument == "--no-cull")
            {
                options.noCull = true;
                continue;
            }

            if (argument == "--reference-capture")
            {
                options.referenceCapture = true;
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
        if (options.direct && options.outputDirectory.has_value())
        {
            throw std::invalid_argument("--direct cannot be combined with --output because no CNJ files are written.");
        }
        if (options.direct && options.unitScale != 1.0f)
        {
            throw std::invalid_argument("--direct cannot be combined with --scale; direct runtime loading preserves glTF units.");
        }
        if (options.referenceCapture && !options.capturePath.has_value())
        {
            throw std::invalid_argument("--reference-capture requires --capture.");
        }
        if (options.outputDirectory.has_value() && options.oracleOutputDirectory.has_value() &&
            NormalizeOutputPath(*options.outputDirectory) ==
                NormalizeOutputPath(*options.oracleOutputDirectory))
        {
            throw std::invalid_argument(
                "--output and --dump-oracle must use different directories.");
        }

        result.options = std::move(options);
        return result;
    }

    void PrintUsage(const char* executableName)
    {
        std::cout
            << "Usage: " << executableName
            << " <model.gltf|model.glb> [options]\n\n"
            << "Loads glTF through CNA's offline CNJ path, or directly with --direct.\n\n"
            << "Options:\n"
            << "  --direct            Load .gltf/.glb directly without generated CNJ files\n"
            << "  --scale <number>    Offline conversion unit scale (default: 1)\n"
            << "  --output <dir>      Keep offline CNJ output in an empty directory\n"
            << "  --dump-oracle <dir> Write deterministic L2-L5 import evidence to an empty directory\n"
            << "  --clip <name>       Select and loop an imported animation clip\n"
            << "  --no-cull           Disable face culling for debugging\n"
            << "  --capture <file>    Save the first rendered frame as PNG and exit\n"
            << "  --reference-capture Capture a clean 512x512 frame for renderer comparison\n\n"
            << "Controls:\n"
            << "  Left mouse drag  Orbit camera\n"
            << "  Mouse wheel      Zoom\n"
            << "  R                Reset camera\n"
            << "  Esc              Exit\n";
    }
}
