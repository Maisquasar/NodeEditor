#pragma once
#include "NodeManager.h"
#include "Actions/Action.h"

#define SAVE_FOLDER "/save/"

#pragma region Dialog
struct Filter
{
    Filter(std::string _name, std::string _spec) : name(std::move(_name)), spec(std::move(_spec)) {}
			
    std::string name;
    // ex : "Text file"
    std::string spec;
    // ex : "txt"
};

std::string SaveDialog(const std::vector<Filter>& filters, const char* defaultPath = nullptr);

std::string OpenDialog(const std::vector<Filter>& filters, const char* defaultPath = nullptr);
#pragma endregion

class LinkManager;
class MainWindow
{
public:
    void Initialize();
    
    void PasteNode() const;

    void Update() const;
    
    void Draw();
    
    void Delete() const;
    void DrawContextMenu(float& zoom, Vec2f& origin, ImVec2 mousePos) const;

    ActionManager& GetActionManager() { return m_actionManager; }

private:
    
    void DrawGrid();

private:
    NodeManager* m_nodeManager = nullptr;

    ActionManager m_actionManager;
    

    struct GridWindow
    {
        Vec2f origin;
        float zoom = 1.f;
    } m_gridWindow;
};
