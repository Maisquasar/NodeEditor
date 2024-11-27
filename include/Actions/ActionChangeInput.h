#pragma once
#include "Action.h"

class ActionChangeInput : public Action 
{
public:
    ActionChangeInput(std::string* input, std::string oldValue, std::string newValue) : m_input(input), m_oldValue(oldValue), m_newValue(newValue) {}

    void Do() override { *m_input = m_newValue; }
    void Undo() override { *m_input = m_oldValue; }

    std::string ToString() override { return "Change Input"; }
private:
    std::string* m_input;
    std::string m_oldValue;
    std::string m_newValue;
};
