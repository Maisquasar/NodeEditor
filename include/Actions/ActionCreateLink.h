#pragma once
#include "Action.h"
#include "NodeManager.h"

class ActionCreateLink : public Action
{
public:
    ActionCreateLink(NodeManager* nodeManager, const LinkRef& link);

    void Do() override;
    void Undo() override;

    std::string ToString() override { return "Create Link"; }

private:
    NodeManager* m_nodeManager = nullptr;
    LinkRef m_link = nullptr;
    
};
