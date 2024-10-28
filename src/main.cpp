#include "Application.h"

int main() {
    
    Application app;
    
    app.Initialize();

    app.Run();

    app.Delete();
    
    return 0;
}