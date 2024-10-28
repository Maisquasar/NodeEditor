#pragma once
#include "NodeManager.h"

class MainWindow
{
public:
    void Initialize();
    
    void Draw();
    
    void Delete();

private:
    
    void DrawGrid();

private:
    std::unique_ptr<NodeManager> m_nodeManager;
};
