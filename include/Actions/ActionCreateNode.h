#pragma once
#include "Action.h"
#include "NodeSystem/Node.h"

class ActionCreateNode : public Action
{
public:
    ActionCreateNode(NodeManager* nodeManager, NodeRef node);

    void Do() override;
    void Undo() override;

    std::string ToString() override { return "Create Node"; }
    bool ShouldUpdateShader() const override { return true; }
public:
    NodeManager* m_nodeManager;
    
    NodeRef m_node;
};
