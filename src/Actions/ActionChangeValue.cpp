#include "Actions/ActionChangeValue.h"

void ActionChangeValue::Do()
{
    *value = newValue;
}

void ActionChangeValue::Undo()
{
    *value = oldValue;
}

std::string ActionChangeValue::ToString()
{
    return "Change Value";
}
