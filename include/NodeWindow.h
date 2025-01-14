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
    virtual void Initialize() = 0;
};

class NodeWindow : public Context
{
public:
    void Initialize() override;
    
    void PasteNode() const;

    void Update() const;
    void Draw();
    void RenderNodePreview(std::shared_ptr<Node> previewNode);
    void Render();
    void ResetActionManager();

    bool Load(const std::filesystem::path& path);
    bool Save(const std::filesystem::path& path);
    bool Save();

    void Delete() const;

    ActionManager& GetActionManager() { return m_actionManager; }

    void UpdateShader();
    void UpdateShaders();

    void ShouldUpdateShader() { m_shouldUpdateShader = true; }
    
    void AddPreviewNode(const UUID& uuid) { m_previewNodes.insert(uuid); }
    void RemovePreviewNode(const UUID& uuid) { m_previewNodes.erase(uuid); }
    void New();
    
    NodeManager* GetNodeManager() const { return m_nodeManager; }

private:
    
    void DrawGrid();
    void DrawInspector() const;

private:
    NodeManager* m_nodeManager = nullptr;

    ActionManager m_actionManager = {};
    bool m_isFocused = false;

    Ref<Shader> m_currentShader;
    Ref<Framebuffer> m_framebuffer;
    bool m_shouldUpdateShader = true;

    std::set<UUID> m_previewNodes;

    Ref<Mesh> m_quad;

    struct GridWindow
    {
        Vec2f origin;
        float zoom = 1.f;
    } m_gridWindow;
};
