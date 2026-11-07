#include "CommandLine.hpp"
#include "ViewerGame.hpp"
#include "System/Exception.hpp"

#include <exception>
#include <iostream>

namespace
{
    void WriteExceptionDiagnostic(const std::exception& error)
    {
        std::cerr << error.what();
        const auto* systemError = dynamic_cast<const System::Exception*>(&error);
        if (systemError == nullptr)
        {
            return;
        }

        const std::exception_ptr inner = systemError->getInnerExceptionProperty();
        if (!inner)
        {
            return;
        }

        std::cerr << ": ";
        try
        {
            std::rethrow_exception(inner);
        }
        catch (const std::exception& cause)
        {
            WriteExceptionDiagnostic(cause);
        }
        catch (...)
        {
            std::cerr << "Unknown exception";
        }
    }
}

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
        std::cerr << "cna-gltf-viewer: ";
        WriteExceptionDiagnostic(error);
        std::cerr << '\n';
        return 1;
    }
}
