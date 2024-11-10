#pragma once
#include <memory>
#include <vector>

class Node;

class NodeTemplateHandler
{
public:
    NodeTemplateHandler() = default;

    static NodeTemplateHandler* Create() { return (s_instance = std::make_unique<NodeTemplateHandler>()).get(); }
    static NodeTemplateHandler* GetInstance() { return s_instance.get(); }

    void Initialize();

    void AddTemplateNode(std::shared_ptr<Node> node);

    std::shared_ptr<Node> CreateFromTemplate(uint32_t templateID) const;

    std::vector<std::shared_ptr<Node>>& GetTemplates() { return m_templateNodes; }

private:
    static std::unique_ptr<NodeTemplateHandler> s_instance;

    std::vector<std::shared_ptr<Node>> m_templateNodes;
};
