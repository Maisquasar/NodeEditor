#include <iostream>

#include "Application.h"

int main(int argc, char** argv) {
    auto app = Application::Create();

    app->Initialize();

    app->Run();

    app->Clean();

    Application::Destroy();

    return 0;
}
