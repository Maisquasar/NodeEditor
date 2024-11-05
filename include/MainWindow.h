#pragma once
#include "NodeManager.h"

struct GridWindow
{
    Vec2f origin;
    float zoom = 1.f;
};

class MainWindow
{
public:
    void Initialize();

    void Update() const;
    
    void Draw();
    
    void Delete();

private:
    
    void DrawGrid();

private:
    std::unique_ptr<NodeManager> m_nodeManager;
    
    GridWindow m_gridWindow;
};
