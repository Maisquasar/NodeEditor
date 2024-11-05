#pragma once
#include "NodeManager.h"

class LinkManager;

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
    NodeManager* m_nodeManager = nullptr;
    
    GridWindow m_gridWindow;
};
