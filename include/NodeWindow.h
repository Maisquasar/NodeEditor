#pragma once
#include <set>

#include "NodeSystem/NodeManager.h"
#include "Actions/Action.h"

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

class Context
{
public:
    virtual ~Context() = default;
    virtual void Initialize() = 0;
};

class NodeWindow : public Context
{
public:
    NodeWindow() = default;
    ~NodeWindow() override = default;

    void InitializeMainNode();
    void Initialize() override;
    
    void PasteNode() const;

    void Update() const;
    void Draw();
    void Render();
    void ResetActionManager();

    bool LoadScene(const std::filesystem::path& path);
    bool SaveScene(const std::filesystem::path& path);
    bool SaveScene();

    void Delete() const;

    ActionManager& GetActionManager() { return m_actionManager; }

    void UpdateShader();
    void UpdateShaders();

    void ShouldUpdateShader() { m_shouldUpdateShader = true; }
    
    void AddPreviewNode(const UUID& uuid) { m_previewNodes.insert(uuid); }
    void RemovePreviewNode(const UUID& uuid) { m_previewNodes.erase(uuid); }
    void NewScene();

    void SetWindowName(const std::string& name) { m_windowName = name; }

    std::string GetWindowName() const { return m_windowName; }
    NodeManager* GetNodeManager() const { return m_nodeManager; }
    void* GetIMGUIWindow() const { return m_ImGuiWindow; }

    Ref<Mesh> GetQuad() const { return m_quad; }
private:
    
    void DrawGrid();
    void DrawInspector() const;

private:
    NodeManager* m_nodeManager = nullptr;

    std::string m_windowName;

    ActionManager m_actionManager = {};
    bool m_isFocused = false;

    Ref<Shader> m_currentShader;
    Ref<Framebuffer> m_framebuffer;
    bool m_shouldUpdateShader = true;

    std::set<UUID> m_previewNodes;

    Ref<Mesh> m_quad;

    void* m_ImGuiWindow = nullptr;

    struct GridWindow
    {
        Vec2f origin;
        float zoom = 1.f;
    } m_gridWindow;
};
