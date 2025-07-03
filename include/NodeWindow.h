#pragma once
#include <set>
#include <unordered_set>

#include "NodeSystem/NodeManager.h"
#include "Actions/Action.h"

class Framebuffer;
class Shader;
class Mesh;

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
    void DrawAndUpdate();
    void Render();
    void ResetActionManager();

    bool LoadScene(const std::filesystem::path& path);
    bool SaveScene(const std::filesystem::path& path);
    bool SaveScene();

    void Delete() const;

    ActionManager& GetActionManager() { return m_actionManager; }

    void UpdateShaders();

    void ShouldUpdateShader(const UUID& nodeToUpdate = UUID_NULL);
    void ShouldUpdateShader(const std::unordered_set<UUID>& nodesToUpdate);

    void AddPreviewNode(const UUID& uuid) { m_previewNodes.insert(uuid); }
    void RemovePreviewNode(const UUID& uuid) { m_previewNodes.erase(uuid); }
    void NewScene();

    void SetWindowName(const std::string& name) { m_windowName = name; }

    std::string GetWindowName() const { return m_windowName; }
    NodeManager* GetNodeManager() const { return m_nodeManager; }
    void* GetIMGUIWindow() const { return m_ImGuiWindow; }

    Ref<Mesh> GetQuad() const { return m_quad; }
    const std::unordered_set<UUID>& GetPreviewNodes() const { return m_previewNodes; }
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
    std::unordered_set<UUID> m_nodesToUpdate;
    
    std::unordered_set<UUID> m_previewNodes;

    Ref<Mesh> m_quad;

    void* m_ImGuiWindow = nullptr;

    struct GridWindow
    {
        Vec2f origin;
        float zoom = 1.f;
    } m_gridWindow;
};
