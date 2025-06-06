﻿#include "Actions/Action.h"

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

void ActionManager::AddAction(const ActionRef& action)
{
    if (m_current->m_undoneActions.size() >= MAX_ACTION_COUNT)
    {
        m_current->m_undoneActions.erase(m_current->m_undoneActions.begin());
    }
    m_current->CleanRedoneActions();
    m_current->m_undoneActions.push_back(action);

    if (auto nodeWindow = dynamic_cast<NodeWindow*>(m_current->m_context))
    {
        nodeWindow->ShouldUpdateShader();
    }
}

void ActionManager::DoAction(const ActionRef& action)
{
    action->Do();
    AddAction(action);
}

void ActionManager::UndoLastAction()
{
    if (!m_current->m_undoneActions.empty())
    {
        m_current->m_undoneActions.back()->Undo();
        m_current->m_redoneActions.push_back(m_current->m_undoneActions.back());
        m_current->m_undoneActions.pop_back();

        if (auto nodeWindow = dynamic_cast<NodeWindow*>(m_current->m_context))
        {
            nodeWindow->ShouldUpdateShader();
        }
    }
}

void ActionManager::RedoLastAction()
{
    if (!m_current->m_redoneActions.empty())
    {
        m_current->m_redoneActions.back()->Do();
        m_current->m_undoneActions.push_back(m_current->m_redoneActions.back());
        m_current->m_redoneActions.pop_back();
        
        if (auto nodeWindow = dynamic_cast<NodeWindow*>(m_current->m_context))
        {
            nodeWindow->ShouldUpdateShader();
        }
    }
}

void ActionManager::SetCurrent(ActionManager* manager)
{
    m_current = manager;
}

void ActionManager::SetContext(Context* context)
{
    m_context = context;
}

void ActionManager::CleanRedoneActions()
{
    m_redoneActions.clear();
}

bool ActionManager::IsInside(const ActionRef& action) const
{
    for (const auto& m_undoneAction : m_undoneActions)
    {
        if (m_undoneAction == action)
        {
            return true;
        }
    }
    return false;
}
