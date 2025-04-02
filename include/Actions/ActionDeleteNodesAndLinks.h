#pragma once
#include "Action.h"
#include "NodeSystem/NodeManager.h"

class ActionDeleteNodesAndLinks : public Action
{
public:
    ActionDeleteNodesAndLinks(NodeManager* nodeManager, const std::vector<NodeWeak>& nodes, const std::vector<LinkRef>& links);

    void Do() override;
    void Undo() override;

    std::string ToString() override { return "Delete Nodes"; }
    bool ShouldUpdateShader() const override { return true; }
private:
    NodeManager* m_nodeManager = nullptr;
    std::vector<NodeRef> m_nodes = {};
    std::vector<LinkRef> m_links = {};
    
};
