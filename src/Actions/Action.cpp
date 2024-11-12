#include "Actions/Action.h"

#include "Application.h"

ActionManager* ActionManager::m_current = nullptr;

ActionManager::~ActionManager()
{
    for (uint32_t i = 0; i < m_undoneActions.size(); i++)
    {
        m_undoneActions[i].reset();
    }
    for (uint32_t i = 0; i < m_redoneActions.size(); i++)
    {
        m_redoneActions[i].reset();
    }
}

void ActionManager::AddAction(Action* action)
{
    if (m_current->IsInside(action))
    {
        return;
    }
    if (m_current->m_undoneActions.size() >= MAX_ACTION_COUNT)
    {
        Action* lastAction = m_current->m_undoneActions.back().get();
        delete lastAction;
        m_current->m_undoneActions.erase(m_current->m_undoneActions.begin());
    }
    m_current->CleanRedoneActions();
    m_current->m_undoneActions.push_back(std::shared_ptr<Action>(action));
}

void ActionManager::SetCurrent(ActionManager* manager)
{
    m_current = manager;
}

void ActionManager::CleanRedoneActions()
{
    m_redoneActions.clear();
}

bool ActionManager::IsInside(Action* action)
{
    for (uint32_t i = 0; i < m_undoneActions.size(); i++)
    {
        if (m_undoneActions[i].get() == action)
        {
            return true;
        }
    }
    return false;
}
