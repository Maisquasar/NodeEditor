#pragma once
#include <iostream>
#include <memory>
#include <vector>

__interface Context;

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

    static void AddAction(const ActionRef& action);

    static void DoAction(const ActionRef& action);

    static void UndoLastAction();

    static void RedoLastAction();

    template<typename T>
    static T* GetLastAction() { return static_cast<T*>(m_current->m_undoneActions.back()); }

    static void UpdateLastAction() { m_current->m_undoneActions.back()->Update(); }

    static ActionManager* GetCurrent() { return m_current; }

    static void SetCurrent(ActionManager* );

    static const std::vector<ActionRef>& GetUndoneActions() { return m_current->m_undoneActions; }

    static const std::vector<ActionRef>& GetRedoneActions() { return m_current->m_redoneActions; }

    
    void SetContext(Context* context);

private:
    void CleanRedoneActions();

    bool IsInside(const ActionRef& action) const;

private:
    static ActionManager* m_current;

    Context* m_context;
    
    std::vector<ActionRef> m_undoneActions;
    std::vector<ActionRef> m_redoneActions;
};