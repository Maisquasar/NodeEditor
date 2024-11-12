#pragma once
#include "Action.h"
#include "NodeManager.h"

class ActionDeleteNodesAndLinks : public Action
{
public:
    ActionDeleteNodesAndLinks(NodeManager* nodeManager, const std::vector<NodeWeakRef>& nodes, const std::vector<LinkWeakRef>& links);

    void Do() override;
    void Undo() override;

    std::string ToString() override { return "Delete Nodes"; }
private:
    NodeManager* m_nodeManager = nullptr;
    std::vector<NodeRef> m_nodes = {};
    std::vector<LinkRef> m_links = {};
    
};
