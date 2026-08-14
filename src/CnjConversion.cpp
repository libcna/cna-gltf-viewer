#include "CnjConversion.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>

namespace CnaGltfViewer
{
    namespace
    {
        namespace fs = std::filesystem;

        [[nodiscard]] std::string QuoteForShell(const fs::path& path)
        {
            const std::string text = path.string();

#ifdef _WIN32
            std::string quoted = "\"";
            for (const char character : text)
            {
                if (character == '\"')
                {
                    quoted += "\\\"";
                }
                else
                {
                    quoted += character;
                }
            }
            return quoted + "\"";
#else
            std::string quoted = "'";
            for (const char character : text)
            {
                if (character == '\'')
                {
                    quoted += "'\\\"'\\\"'";
                }
                else
                {
                    quoted += character;
                }
            }
            return quoted + "'";
#endif
        }

        [[nodiscard]] fs::path CreateTemporaryOutputDirectory()
        {
            std::error_code error;
            const fs::path root = fs::temp_directory_path(error) / "cna-gltf-viewer";
            if (error)
            {
                throw std::runtime_error("Could not determine the system temporary directory: " + error.message());
            }

            fs::create_directories(root, error);
            if (error)
            {
                throw std::runtime_error("Could not create the temporary conversion root: " + error.message());
            }

            const auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
            for (unsigned int suffix = 0; suffix != 1000; ++suffix)
            {
                const fs::path candidate = root / ("import-" + std::to_string(seed) + "-" + std::to_string(suffix));
                if (fs::create_directory(candidate, error))
                {
                    return candidate;
                }
                if (error && error != std::errc::file_exists)
                {
                    throw std::runtime_error("Could not create a temporary conversion directory: " + error.message());
                }
                error.clear();
            }

            throw std::runtime_error("Could not allocate a unique temporary conversion directory.");
        }

        [[nodiscard]] fs::path PrepareOutputDirectory(const ViewerOptions& options)
        {
            if (!options.outputDirectory.has_value())
            {
                return CreateTemporaryOutputDirectory();
            }

            std::error_code error;
            const fs::path output = fs::absolute(*options.outputDirectory, error);
            if (error)
            {
                throw std::runtime_error("Could not resolve the output directory: " + error.message());
            }

            if (fs::exists(output, error))
            {
                if (error)
                {
                    throw std::runtime_error("Could not inspect the output directory: " + error.message());
                }
                if (!fs::is_directory(output, error))
                {
                    throw std::runtime_error("The --output path exists but is not a directory.");
                }
                if (!fs::is_empty(output, error))
                {
                    if (error)
                    {
                        throw std::runtime_error("Could not inspect the output directory: " + error.message());
                    }
                    throw std::runtime_error("The --output directory must be empty.");
                }
            }
            else
            {
                fs::create_directories(output, error);
                if (error)
                {
                    throw std::runtime_error("Could not create the output directory: " + error.message());
                }
            }

            return output;
        }

        [[nodiscard]] bool IsModelCnj(const fs::path& path)
        {
            std::ifstream input(path);
            if (!input)
            {
                throw std::runtime_error("Could not read generated CNJ file: " + path.string());
            }

            std::ostringstream text;
            text << input.rdbuf();
            return text.str().find("\"type\": \"Model\"") != std::string::npos;
        }

        [[nodiscard]] std::vector<fs::path> FindModelAssets(const fs::path& outputDirectory)
        {
            std::vector<fs::path> assets;
            std::error_code error;
            const fs::directory_iterator end;
            fs::directory_iterator iterator(outputDirectory, error);
            if (error)
            {
                throw std::runtime_error("Could not inspect converter output: " + error.message());
            }
            for (; iterator != end; iterator.increment(error))
            {
                if (error)
                {
                    throw std::runtime_error("Could not inspect converter output: " + error.message());
                }
                const fs::directory_entry& entry = *iterator;
                if (!entry.is_regular_file() || entry.path().extension() != ".cnj")
                {
                    continue;
                }
                if (IsModelCnj(entry.path()))
                {
                    assets.push_back(entry.path().stem());
                }
            }

            std::sort(assets.begin(), assets.end());
            return assets;
        }

        struct TemporaryOutputGuard
        {
            fs::path path;
            bool armed = false;

            ~TemporaryOutputGuard()
            {
                if (!armed)
                {
                    return;
                }
                std::error_code ignored;
                fs::remove_all(path, ignored);
            }
        };
    }

    ConvertedScene CnjConverter::Convert(const ViewerOptions& options)
    {
        namespace fs = std::filesystem;

        std::error_code error;
        const fs::path input = fs::absolute(options.inputPath, error);
        if (error || !fs::is_regular_file(input, error))
        {
            throw std::runtime_error("Input glTF file does not exist or is not a regular file: " + options.inputPath.string());
        }

        const fs::path output = PrepareOutputDirectory(options);
        TemporaryOutputGuard failureCleanup{
            .path = output,
            .armed = !options.outputDirectory.has_value(),
        };
        const fs::path converter(CNA_GLTF_TO_CNJ_TOOL_PATH);
        if (!fs::is_regular_file(converter, error))
        {
            throw std::runtime_error("CNA glTF-to-CNJ converter was not found: " + converter.string());
        }

        const std::string command = QuoteForShell(converter) + " " + QuoteForShell(input) + " " +
                                    QuoteForShell(output) + " scene " + std::to_string(options.unitScale);
        std::cout << "Converting " << input << " to CNJ in " << output << "...\n";
        const int exitCode = std::system(command.c_str());
        if (exitCode != 0)
        {
            throw std::runtime_error("CNA glTF-to-CNJ conversion failed with exit code " + std::to_string(exitCode) + ".");
        }

        std::vector<fs::path> models = FindModelAssets(output);
        if (models.empty())
        {
            throw std::runtime_error("CNA conversion completed but produced no Model CNJ asset.");
        }

        std::cout << "Loaded " << models.size() << " CNJ model asset(s).\n";
        ConvertedScene result{
            .outputDirectory = output,
            .modelAssets = std::move(models),
            .temporaryOutput = failureCleanup.armed,
        };
        failureCleanup.armed = false;
        return result;
    }

    void CnjConverter::DumpOracle(const ViewerOptions& options)
    {
        if (!options.oracleOutputDirectory.has_value())
        {
            return;
        }

        namespace fs = std::filesystem;
        std::error_code error;
        const fs::path input = fs::absolute(options.inputPath, error);
        if (error || !fs::is_regular_file(input, error))
        {
            throw std::runtime_error(
                "Input glTF file does not exist or is not a regular file: " +
                options.inputPath.string());
        }

        const fs::path output = fs::absolute(*options.oracleOutputDirectory, error);
        if (error)
        {
            throw std::runtime_error(
                "Could not resolve the oracle output directory: " + error.message());
        }

        const fs::path converter(CNA_GLTF_TO_CNJ_TOOL_PATH);
        if (!fs::is_regular_file(converter, error))
        {
            throw std::runtime_error(
                "CNA glTF-to-CNJ converter was not found: " + converter.string());
        }

        const std::string command = QuoteForShell(converter) + " --dump-oracle " +
                                    QuoteForShell(input) + " " + QuoteForShell(output);
        std::cout << "Writing L2-L5 oracle evidence for " << input << " to " << output
                  << "...\n";
        const int exitCode = std::system(command.c_str());
        if (exitCode != 0)
        {
            throw std::runtime_error(
                "CNA glTF oracle dump failed with exit code " +
                std::to_string(exitCode) + ".");
        }

        const fs::path oracle = output / "oracle.json";
        if (!fs::is_regular_file(oracle, error))
        {
            throw std::runtime_error(
                "CNA glTF oracle dump completed but did not produce: " + oracle.string());
        }
        std::cout << "Oracle evidence written to " << oracle << ".\n";
    }
}
