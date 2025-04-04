#pragma once
#include <Maths.h>

#include "Action.h"

class RerouteNodeNamedManager;
class RerouteNodeNamed;

class ActionRenameRerouteNode : public Action
{
public:
    ActionRenameRerouteNode(RerouteNodeNamed* node, const std::string& oldName, const std::string& newName) : m_node(node), m_oldName(oldName), m_newName(newName) {}
    void Do() override;
    void Undo() override;
    
    std::string ToString() override { return "Rename Reroute Node"; }
    bool ShouldUpdateShader() const override { return false; }
private:
    RerouteNodeNamed* m_node = nullptr;
    std::string m_oldName;
    std::string m_newName;
};

class ActionChangeColorRerouteNode : public Action
{
public:
    ActionChangeColorRerouteNode(RerouteNodeNamedManager* manager, const std::string& name, Vec4f prevColor, Vec4f newColor);
    void Do() override;
    void Undo() override;
    
    std::string ToString() override { return "Change Color Reroute Node"; }
    bool ShouldUpdateShader() const override { return false; }
private:
    RerouteNodeNamedManager* m_rerouteNodeNamedManager = nullptr;
    std::string m_name;
    Vec4f m_prevColor;
    Vec4f m_newColor;
};