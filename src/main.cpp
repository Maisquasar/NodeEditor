#include "Application.h"

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif // _WIN32

int Main()
{
    Application app;
    
    app.Initialize();

    app.Run();

    app.Delete();
    
    return 0;
}

int main() {
    
#ifdef _WIN32
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //  TODO: Remove Comments To Break on leaks
    // |
    // V
    //_CrtSetBreakAlloc(863);
#endif

    return Main();
}