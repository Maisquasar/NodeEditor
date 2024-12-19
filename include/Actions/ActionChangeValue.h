#pragma once
#include "Action.h"
#include "NodeSystem/NodeManager.h"

class ActionChangeValue : public Action
{
public:
    ActionChangeValue(Vec4f oldValue, Vec4f newValue, Vec4f* value) : oldValue(oldValue), newValue(newValue), value(value) {};
    
    void Do() override;
    void Undo() override;
    std::string ToString() override;
protected:
    Vec4f oldValue;
    Vec4f newValue;
    Vec4f* value;
};