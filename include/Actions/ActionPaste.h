#pragma once
#include "Action.h"
#include "NodeSystem/NodeManager.h"

class NodeManager;

class ActionPaste : public Action
{
public:
    ActionPaste(NodeManager* nodeManager, float zoom, const Vec2f& origin, const Vec2f& mousePos, const char* clipboardText) :
    m_clipboardText(clipboardText), m_zoom(zoom), m_origin(origin), m_mousePos(mousePos), m_nodeManager(nodeManager)
    {
    }

    void Do() override;

    void Undo() override;

    std::string ToString() override { return "Paste"; }

private:
    const char* m_clipboardText = nullptr;
    float m_zoom = 1.f;
    Vec2f m_origin;
    Vec2f m_mousePos;
    NodeManager* m_nodeManager = nullptr;

    std::vector<NodeRef> m_pastedNodes;
    std::vector<LinkRef> m_pastedLinks;
};
