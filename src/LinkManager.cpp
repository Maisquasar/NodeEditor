#include "LinkManager.h"

#include <utility>

#include "NodeManager.h"

void LinkManager::DrawLinks(float zoom, const Vec2f& origin) const
{
    auto drawList = ImGui::GetWindowDrawList();
    NodeManager* nodeManager = NodeManager::Get();

    if (LinkRef selectedLink = nodeManager->GetSelectedLink().lock())
    {
        NodeRef fromNode = nodeManager->GetNode(selectedLink->fromNodeIndex).lock();
        NodeRef toNode = nodeManager->GetNode(selectedLink->toNodeIndex).lock();
        
        assert(toNode != nullptr && fromNode != nullptr && "Node not found");
        
        Vec2f inputPosition = fromNode->GetOutputPosition(selectedLink->fromOutputIndex, origin, zoom);
        Vec2f outputPosition = toNode->GetInputPosition(selectedLink->toInputIndex, origin, zoom);
        
        Vec2f controlPoint1 = inputPosition + Vec2f(50.0f, 0.0f) * zoom;
        Vec2f controlPoint2 = outputPosition - Vec2f(50.0f, 0.0f) * zoom;
        
        drawList->AddBezierCubic(inputPosition, controlPoint1, controlPoint2, outputPosition, IM_COL32(255, 255, 0, 255), 3 * zoom);
    }
    
    for (const LinkRef& link : m_links)
    {
        NodeRef fromNode = nodeManager->GetNode(link->fromNodeIndex).lock();
        NodeRef toNode = nodeManager->GetNode(link->toNodeIndex).lock();
        
        assert(toNode != nullptr && fromNode != nullptr && "Node not found");
        
        Vec2f inputPosition = fromNode->GetOutputPosition(link->fromOutputIndex, origin, zoom);
        Vec2f outputPosition = toNode->GetInputPosition(link->toInputIndex, origin, zoom);
        
        Vec2f controlPoint1 = inputPosition + Vec2f(50.0f, 0.0f) * zoom;
        Vec2f controlPoint2 = outputPosition - Vec2f(50.0f, 0.0f) * zoom;
        
        drawList->AddBezierCubic(inputPosition, controlPoint1, controlPoint2, outputPosition, IM_COL32(255, 255, 255, 255), 2 * zoom);
    }
}

void LinkManager::CreateLink(const NodeRef& fromNode, const uint32_t fromOutput, const NodeRef& toNode, const uint32_t toOutput)
{
    CreateLink(fromNode->GetUUID(), fromOutput, toNode->GetUUID(), toOutput);
}

void LinkManager::CreateLink(UUID fromNodeIndex, const uint32_t fromOutputIndex, UUID toNodeIndex, const uint32_t toOutputIndex)
{
    m_links.push_back(std::make_shared<Link>(std::move(fromNodeIndex), fromOutputIndex, std::move(toNodeIndex), toOutputIndex));
}

void LinkManager::AddLink(Link link)
{
    m_links.push_back(std::make_shared<Link>(std::move(link)));
}

void LinkManager::RemoveLink(const NodeRef& fromNode, const uint32_t fromOutput, const NodeRef& toNode, const uint32_t toOutput)
{
    RemoveLink(fromNode->GetUUID(), fromOutput, toNode->GetUUID(), toOutput);
}

void LinkManager::RemoveLink(const UUID& fromNodeIndex, const uint32_t fromOutputIndex, const UUID& toNodeIndex, const uint32_t toOutputIndex)
{
    for (size_t i = 0; i < m_links.size(); i++)
    {
        if (m_links[i]->fromNodeIndex == fromNodeIndex &&
            m_links[i]->fromOutputIndex == fromOutputIndex &&
            m_links[i]->toNodeIndex == toNodeIndex &&
            m_links[i]->toInputIndex == toOutputIndex)
        {
            m_links.erase(m_links.begin() + i);
            break;
        }
    }
}

void LinkManager::RemoveLink(const LinkWeakRef& link)
{
    LinkRef linkRef = link.lock();
    for (size_t i = 0; i < m_links.size(); i++)
    {
        if (m_links[i] == linkRef)
        {
            m_links.erase(m_links.begin() + i);
            break;
        }
    }
}

void LinkManager::RemoveLink(const InputRef& input)
{
    for (size_t i = 0; i < m_links.size(); i++)
    {
        if (m_links[i]->toNodeIndex == input->parentUUID && m_links[i]->toInputIndex == input->index)
        {
            m_links.erase(m_links.begin() + i);
            break;
        }
    }
}

void LinkManager::RemoveLinks(const OutputRef& output)
{
    for (size_t i = 0; i < m_links.size(); i++)
    {
        if (m_links[i]->fromNodeIndex == output->parentUUID && m_links[i]->fromOutputIndex == output->index)
        {
            m_links.erase(m_links.begin() + i);
            i--;
        }
    }
}

void LinkManager::RemoveLinks(const NodeRef& node)
{
    for (size_t i = 0; i < m_links.size(); i++)
    {
        if (m_links[i]->fromNodeIndex == node->GetUUID() || m_links[i]->toNodeIndex == node->GetUUID())
        {
            m_links.erase(m_links.begin() + i);
            i--;
        }
    }
}

bool LinkManager::IsPointHoverLineSegment(Vec2f pointPosition, Vec2f fromPosition, Vec2f toPosition, float threshold)
{
    Vec2f lineVec = toPosition - fromPosition;
    Vec2f pointVec = pointPosition - fromPosition;

    float lineLengthSq = lineVec.Dot(lineVec);
    float projection = pointVec.Dot(lineVec) / lineLengthSq;

    if (projection < 0.0f || projection > 1.0f) {
        return false;
    }

    Vec2f closestPoint = fromPosition + lineVec * projection;
    Vec2f hoverVec = pointPosition - closestPoint;

    return hoverVec.Length() <= threshold;
}

LinkWeakRef LinkManager::GetLinkWithOutput(const UUID& uuid, uint32_t index) const
{
    for (const LinkRef& link : m_links)
    {
        if (link->fromNodeIndex == uuid && link->fromOutputIndex == index)
        {
            return link;
        }
    }
    return {};
}

LinkWeakRef LinkManager::GetLinkClicked(float zoom, const Vec2f& origin, const Vec2f& mousePos) const
{
    for (const LinkRef& link : m_links)
    {
        NodeRef fromNode = NodeManager::Get()->GetNode(link->fromNodeIndex).lock();
        NodeRef toNode = NodeManager::Get()->GetNode(link->toNodeIndex).lock();
        
        Vec2f inputPosition = fromNode->GetOutputPosition(link->fromOutputIndex, origin, zoom);
        Vec2f outputPosition = toNode->GetInputPosition(link->toInputIndex, origin, zoom);
        
        if (IsPointHoverLineSegment(mousePos, inputPosition, outputPosition, 3.0f * zoom))
        {
            return link;
        }
    }
    return {};
}
