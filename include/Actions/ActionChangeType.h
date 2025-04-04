#pragma once
#include "Action.h"
#include "NodeSystem/NodeManager.h"

class CustomNode;
class ParamNode;

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

    std::vector<Link> m_prevLinks = {};
};

class ActionChangeTypeCustom : public Action
{
public:
    ActionChangeTypeCustom(CustomNode* node, StreamRef stream, Type type, Type oldType);
    void Do() override;
    void Undo() override;
    std::string ToString() override { return "Change Type Custom"; }
    bool ShouldUpdateShader() const override { return true; }
private:
    CustomNode* m_customNode = nullptr;
    StreamRef m_stream = nullptr;
    
    Type m_type;
    Type m_oldType;
    
    std::vector<Link> m_prevLinks = {};
};