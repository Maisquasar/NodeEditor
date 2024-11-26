#pragma once
#include <Maths.h>
#include <unordered_map>

#include "Action.h"
#include "NodeSystem/Node.h"

class Node;

struct MoveNodeData
{
    Vec2f position;
    Vec2f oldPosition;
};

// Hash function for std::weak_ptr
struct WeakPtrHash {
    template <typename T>
    std::size_t operator()(const std::weak_ptr<T>& wp) const {
        if (auto sp = wp.lock()) {
            return std::hash<std::shared_ptr<T>>()(sp);
        }
        return 0;
    }
};

// Equality function for std::weak_ptr
struct WeakPtrEqual {
    template <typename T>
    bool operator()(const std::weak_ptr<T>& lhs, const std::weak_ptr<T>& rhs) const {
        return !lhs.owner_before(rhs) && !rhs.owner_before(lhs);
    }
};

class ActionMoveNodes : public Action
{
public:
    explicit ActionMoveNodes(const std::vector<NodeWeakRef>& nodes);
    
    void Do() override;
    void Undo() override;
    void Update() override;

    std::string ToString() override { return "Move Nodes"; }

private:
    std::unordered_map<NodeWeakRef, MoveNodeData, WeakPtrHash, WeakPtrEqual> m_positions;
};
