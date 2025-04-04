#pragma once
#include "Action.h"
#include "NodeSystem/NodeManager.h"
#include "NodeSystem/ParamNode.h"

class ActionChangeValue : public Action
{
public:
    ActionChangeValue(Vec4f oldValue, Vec4f newValue, Vec4f* value) : oldValue(oldValue), newValue(newValue), value(value) {};
    
    void Do() override;
    void Undo() override;
    std::string ToString() override;
    bool ShouldUpdateShader() const override { return true; }
protected:
    Vec4f oldValue;
    Vec4f newValue;
    Vec4f* value;
};

class ActionChangePreviewValue : public Action
{
public:
    ActionChangePreviewValue(ParamNode* paramNode,const Vec4f& oldValue,const Vec4f& newValue, const std::filesystem::path& oldTexturePath, const std::filesystem::path& newTexturePath);
    void Do() override;
    void Undo() override;
    std::string ToString() override { return "Change Preview Value"; }
    bool ShouldUpdateShader() const override { return true; }
private:
    ParamNode* m_paramNode;
    Vec4f m_oldValue;
    Vec4f m_newValue;
    std::filesystem::path m_oldTexturePath;  
    std::filesystem::path m_newTexturePath;  
};
