#pragma once
#include "NodeSystem/NodeManager.h"
#include "Actions/Action.h"

#define SAVE_FOLDER "saves/"
#define EDITOR_FILE_NAME "editor.settings"
#define TEMP_FOLDER "tmp/"

#pragma region Dialog
class Framebuffer;
class Shader;
class Mesh;

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

__interface Context
{
    virtual void Initialize() = 0;
};

class NodeWindow : public Context
{
public:
    void Initialize() override;
    
    void PasteNode() const;

    void Update() const;
    void Draw();
    void Render();
    void ResetActionManager();

    void Delete() const;
    static void DrawMainDock();
    void DrawContextMenu(float& zoom, Vec2f& origin, ImVec2 mousePos);

    ActionManager& GetActionManager() { return m_actionManager; }

    void SetOpenContextMenu(bool shouldOpen);

    bool IsContextMenuOpen() const { return m_contextOpen; }

    Vec2f GetMousePosOnContext() const { return m_mousePosOnContext; }

    void UpdateShader();

    void ShouldUpdateShader() { m_shouldUpdateShader = true; }
private:
    
    void DrawGrid();
    void DrawInspector() const;
    void DrawMainBar();

    void WriteEditorFile(const std::string& path) const;
    void LoadEditorFile(const std::string& path) const;

private:
    NodeManager* m_nodeManager = nullptr;

    ActionManager m_actionManager = {};
    bool m_isFocused = false;

    Vec2f m_mousePosOnContext;
    bool m_focusInput = false;
    int m_shouldOpenContextMenu = -1;
    bool m_contextOpen = false;

    Ref<Mesh> m_quad;
    Ref<Shader> m_currentShader;
    Ref<Framebuffer> m_framebuffer;
    bool m_shouldUpdateShader = true;

    struct GridWindow
    {
        Vec2f origin;
        float zoom = 1.f;
    } m_gridWindow;
};
