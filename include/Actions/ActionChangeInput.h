#pragma once
#include "Action.h"
#include "NodeSystem/CustomNode.h"

class ActionChangeStreamNameParam : public Action
{
public:
    ActionChangeStreamNameParam(ParamNode* node, std::string* input, const std::string& oldValue, const std::string& newValue);
    void Do() override;
    void Undo() override;
    std::string ToString() override { return "Change Input Param"; }
    bool ShouldUpdateShader() const override { return true; }
private:
    ParamNode* m_node;
    std::string* m_input;
    std::string m_oldValue;
    std::string m_newValue;
};

class ActionChangeStreamNameCustom : public Action 
{
public:
    ActionChangeStreamNameCustom(CustomNode* node, std::string* input, std::string oldValue, std::string newValue);
    void Do() override;
    void Undo() override;
    std::string ToString() override { return "Change Input Custom"; }
    bool ShouldUpdateShader() const override { return true; }
private:
    CustomNode* m_node;
    std::string* m_input;
    std::string m_oldValue;
    std::string m_newValue;
};
