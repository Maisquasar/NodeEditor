#pragma once
#include "Action.h"
#include "NodeSystem/NodeManager.h"

class CustomNode;
class ParamNode;

class ActionChangeType : public Action
{
public:
    ActionChangeType(ParamNode* node, Type type, Type oldType);
    ActionChangeType(CustomNode* node, InputRef input, Type type, Type oldType);
    ActionChangeType(CustomNode* node, OutputRef output, Type type, Type oldType);

    void Do() override;
    void Undo() override;

    std::string ToString() override { return "Change Type"; }
    bool ShouldUpdateShader() const override { return true; }

private:
    ParamNode* m_paramNode = nullptr;
    CustomNode* m_customNode = nullptr;
    InputRef m_input = nullptr;
    OutputRef m_output = nullptr;
    
    Type m_type;
    Type m_oldType;
    
    std::vector<Link> m_link;
};

class ActionChangeTypeParam : public Action
{
public:
    ActionChangeTypeParam(ParamNodeManager* paramNodeManager, const std::string& paramName, Type type, Type oldType);
    void Do() override;
    void Undo() override;
    std::string ToString() override { return "Change Type Param"; }
    bool ShouldUpdateShader() const override { return true; }
private:
    ParamNodeManager* m_paramNodeManager = nullptr;
    std::string m_paramName;
    Type m_type;
    Type m_oldType;

    std::vector<LinkRef> m_prevLinks = {};
};