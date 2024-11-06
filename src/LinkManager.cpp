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
        
        Vec2f controlPoint1 = inputPosition + Vec2f(m_controlDistanceX, 0.0f) * zoom;
        Vec2f controlPoint2 = outputPosition - Vec2f(m_controlDistanceX, 0.0f) * zoom;
        
        drawList->AddBezierCubic(inputPosition, controlPoint1, controlPoint2, outputPosition, IM_COL32(255, 255, 0, 255), 3 * zoom, m_bezierSegmentCount);
    }
    
    for (const LinkRef& link : m_links)
    {
        NodeRef fromNode = nodeManager->GetNode(link->fromNodeIndex).lock();
        NodeRef toNode = nodeManager->GetNode(link->toNodeIndex).lock();
        
        assert(toNode != nullptr && fromNode != nullptr && "Node not found");
        
        Vec2f inputPosition = fromNode->GetOutputPosition(link->fromOutputIndex, origin, zoom);
        Vec2f outputPosition = toNode->GetInputPosition(link->toInputIndex, origin, zoom);
        
        Vec2f controlPoint1 = inputPosition + Vec2f(m_controlDistanceX, 0.0f) * zoom;
        Vec2f controlPoint2 = outputPosition - Vec2f(m_controlDistanceX, 0.0f) * zoom;
        
        drawList->AddBezierCubic(inputPosition, controlPoint1, controlPoint2, outputPosition, IM_COL32(255, 255, 255, 255), 2 * zoom, m_bezierSegmentCount);
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
    for (uint32_t i = 0; i < m_links.size(); i++)
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
    for (uint32_t i = 0; i < m_links.size(); i++)
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
    for (uint32_t i = 0; i < m_links.size(); i++)
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
    for (uint32_t i = 0; i < m_links.size(); i++)
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
    for (uint32_t i = 0; i < m_links.size(); i++)
    {
        if (m_links[i]->fromNodeIndex == node->GetUUID() || m_links[i]->toNodeIndex == node->GetUUID())
        {
            m_links.erase(m_links.begin() + i);
            i--;
        }
    }
}

bool LinkManager::CanCreateLink(const Link& link) const
{
    if (link.fromNodeIndex == UUID_NULL || link.toNodeIndex == UUID_NULL)
        return false;
    NodeManager* nodeManager = NodeManager::Get();
    
    NodeRef fromNode = nodeManager->GetNode(link.fromNodeIndex).lock();
    if (!fromNode)
        return false;
    OutputRef fromOutput = fromNode->GetOutput(link.fromOutputIndex);

    
    NodeRef toNode = nodeManager->GetNode(link.toNodeIndex).lock();
    if (!toNode)
        return false;
    InputRef toInput = toNode->GetInput(link.toInputIndex);

    // Check if nodes are valid, not the same and the same input and output type
    if (!fromOutput || !toInput || fromNode == toNode || fromOutput->type != toInput->type)
        return false;

    // Check if input is already connected
    for (const LinkRef& inLink : m_links)
    {
        if (inLink->toNodeIndex == link.toNodeIndex && inLink->toInputIndex == link.toInputIndex)
            return false;
    }
    
    return true;
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

bool LinkManager::IsPointHoverBezier(Vec2f pointPosition, Vec2f inputPosition, Vec2f controlPoint1, Vec2f controlPoint2,
                                     Vec2f outputPosition, float threshold, int numSamples)
{
    for (int i = 0; i <= numSamples; ++i)
    {
        float t = i / static_cast<float>(numSamples);
        
        // Calculate the point on the cubic Bezier curve for this t
        Vec2f bezierPoint = 
            inputPosition * (1 - t) * (1 - t) * (1 - t) +
            controlPoint1 * 3 * (1 - t) * (1 - t) * t +
            controlPoint2 * 3 * (1 - t) * t * t +
            outputPosition* t * t * t;
        
        // Calculate distance from the point to the Bezier point
        const float distance = (pointPosition - bezierPoint).Length();
        
        // Check if this distance is within the threshold
        if (distance < threshold)
        {
            return true;
        }
    }
    return false;
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

std::vector<LinkWeakRef> LinkManager::GetLinksWithInput(const UUID& uuid, uint32_t index) const
{
    std::vector<LinkWeakRef> links;
    for (const LinkRef& link : m_links)
    {
        if (link->toNodeIndex == uuid && link->toInputIndex == index)
        {
            links.push_back(link);
        }
    }
    return links;
}

bool LinkManager::HasLink(const OutputRef& output) const
{
    for (const LinkRef& link : m_links)
    {
        if (link->fromNodeIndex == output->parentUUID && link->fromOutputIndex == output->index)
        {
            return true;
        }
    }
    return false;
}

bool LinkManager::HasLink(const InputRef& input) const
{
    for (const LinkRef& link : m_links)
    {
        if (link->toNodeIndex == input->parentUUID && link->toInputIndex == input->index)
        {
            return true;
        }
    }
    return false;
}

LinkWeakRef LinkManager::GetLinkClicked(float zoom, const Vec2f& origin, const Vec2f& mousePos) const
{
    for (const LinkRef& link : m_links)
    {
        NodeRef fromNode = NodeManager::Get()->GetNode(link->fromNodeIndex).lock();
        NodeRef toNode = NodeManager::Get()->GetNode(link->toNodeIndex).lock();
        
        Vec2f inputPosition = fromNode->GetOutputPosition(link->fromOutputIndex, origin, zoom);
        Vec2f outputPosition = toNode->GetInputPosition(link->toInputIndex, origin, zoom);
        
        if (IsPointHoverBezier(mousePos, inputPosition, inputPosition + Vec2f(m_controlDistanceX, 0.0f) * zoom,
                               outputPosition - Vec2f(m_controlDistanceX, 0.0f) * zoom, outputPosition, 3.f * zoom,
                               m_bezierSegmentCount))
        {
            return link;
        }
    }
    return {};
}
