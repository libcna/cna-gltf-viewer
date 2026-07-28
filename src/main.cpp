#include "CommandLine.hpp"
#include "ViewerGame.hpp"

#include <exception>
#include <iostream>

int main(const int argc, char* argv[])
{
    try
    {
        const CnaGltfViewer::CommandLineResult commandLine = CnaGltfViewer::ParseCommandLine(argc, argv);
        if (commandLine.showHelp)
        {
            CnaGltfViewer::PrintUsage(argv[0]);
            return 0;
        }

        CnaGltfViewer::ViewerGame game(*commandLine.options);
        game.Run();
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "cna-gltf-viewer: " << error.what() << '\n';
        return 1;
    }
}
