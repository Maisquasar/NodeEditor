#pragma once
#include <iostream>
#include <memory>
#include <vector>

class Action
{
public:
    virtual void Do() = 0;
    virtual void Undo() = 0;
    virtual void Update() {}
    virtual std::string ToString() = 0;
    virtual ~Action() = default;
};

using ActionRef = std::shared_ptr<Action>;


#define MAX_ACTION_COUNT 10
class ActionManager
{
public:
    ~ActionManager();

    static void AddAction(Action* action);

    static void DoAction(Action* action)
    {
        action->Do();
        AddAction(action);
    }

    static void UndoLastAction()
    {
        if (!m_current->m_undoneActions.empty())
        {
            m_current->m_undoneActions.back()->Undo();
            m_current->m_redoneActions.push_back(m_current->m_undoneActions.back());
            m_current->m_undoneActions.pop_back();
        }
    }

    static void RedoLastAction()
    {
        if (!m_current->m_redoneActions.empty())
        {
            m_current->m_redoneActions.back()->Do();
            m_current->m_undoneActions.push_back(m_current->m_redoneActions.back());
            m_current->m_redoneActions.pop_back();
        }
    }

    template<typename T>
    static T* GetLastAction() { return static_cast<T*>(m_current->m_undoneActions.back()); }

    static void UpdateLastAction() { m_current->m_undoneActions.back()->Update(); }

    static ActionManager* GetCurrent() { return m_current; }

    static void SetCurrent(ActionManager* );

    static const std::vector<ActionRef>& GetUndoneActions() { return m_current->m_undoneActions; }

private:
    void CleanRedoneActions();

    bool IsInside(Action* action);

private:
    static ActionManager* m_current;
    
    std::vector<ActionRef> m_undoneActions;
    std::vector<ActionRef> m_redoneActions;
};