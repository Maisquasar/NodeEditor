#pragma once
#include "NodeManager.h"
#include "Actions/Action.h"

#define SAVE_FOLDER "/save/"
#define EDITOR_FILE_NAME "editor.settings"

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
    static void DrawMainDock();
    void DrawContextMenu(float& zoom, Vec2f& origin, ImVec2 mousePos);

    ActionManager& GetActionManager() { return m_actionManager; }

private:
    
    void DrawGrid();
    void DrawInspector() const;
    void DrawMainBar();

    void WriteEditorFile(const std::string& path) const;
    void LoadEditorFile(const std::string& path) const;

private:
    NodeManager* m_nodeManager = nullptr;

    ActionManager m_actionManager;

    Vec2f m_mousePosOnContext;
    bool m_focusInput = false;

    struct GridWindow
    {
        Vec2f origin;
        float zoom = 1.f;
    } m_gridWindow;
};
