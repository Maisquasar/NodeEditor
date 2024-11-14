#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Node.h"

class Node;

struct NodeMethodInfo
{
    NodeMethodInfo() = default;
    NodeMethodInfo(NodeRef ref) : node(ref) {}
    NodeMethodInfo(NodeRef ref, std::string _formatString) : node(ref), formatString(std::move(_formatString)) {}
    NodeMethodInfo(std::string _formatString) : formatString(std::move(_formatString)) {}
    

    NodeRef node;
    std::string formatString;
};

using TemplateList = std::vector<NodeMethodInfo>;
class NodeTemplateHandler
{
public:
    NodeTemplateHandler() = default;

    static NodeTemplateHandler* Create() { return (s_instance = std::make_unique<NodeTemplateHandler>()).get(); }
    static NodeTemplateHandler* GetInstance() { return s_instance.get(); }

    void Initialize();

    void AddTemplateNode(const NodeMethodInfo& info);

    static std::shared_ptr<Node> CreateFromTemplate(uint32_t templateID);

    TemplateList& GetTemplates() { return m_templateNodes; }

private:
    static std::unique_ptr<NodeTemplateHandler> s_instance;

    TemplateList m_templateNodes;
};
