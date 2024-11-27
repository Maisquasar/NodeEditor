#pragma once
#include "Action.h"
#include "NodeSystem/NodeManager.h"

class ParamNode;

class ActionChangeType : public Action
{
public:
    ActionChangeType(ParamNode* node, Type type, Type oldType);;

    void Do() override;
    void Undo() override;

    std::string ToString() override { return "Change Type"; }

private:
    ParamNode* m_node;
    Type m_type;
    Type m_oldType;
    std::vector<Link> m_link;
};
