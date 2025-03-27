#include <iostream>

#ifdef _WIN32
#include <crtdbg.h>
#endif

#include "Application.h"

int Main(int argc, char** argv)
{
    auto app = Application::Create();

    app->Initialize();

    app->Run();

    app->Clean();

    Application::Destroy();

    return 0;
}

int main(int argc, char** argv) {
#ifdef _WIN32
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    // TODO: Remove Comments To Break on leaks
    // |
    // V
    // _CrtSetBreakAlloc(0);
#endif
    
    int result = Main(argc, argv);

    return result;
}