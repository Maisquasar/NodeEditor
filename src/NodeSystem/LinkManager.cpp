#include "NodeSystem/LinkManager.h"

#include <CppSerializer.h>
#include <utility>

#include "NodeSystem/NodeManager.h"
namespace Utils
{
    // Utility function to get a point on a cubic Bezier curve for a given t (0 <= t <= 1)
    Vec2f CubicBezierPoint(const Vec2f& p0, const Vec2f& p1, const Vec2f& p2, const Vec2f& p3, float t)
    {
        float u = 1 - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        Vec2f point = p0 * uuu; // u^3 * p0
        point += p1* 3 * uu * t; // 3 * u^2 * t * p1
        point += p2 * 3 * u * tt; // 3 * u * t^2 * p2
        point += p3 * ttt; // t^3 * p3

        return point;
    }

    // Helper function to check if a line segment intersects the rectangle
    bool LineIntersectsRect(const Vec2f& start, const Vec2f& end, const Vec2f& rectMin, const Vec2f& rectMax)
    {
        // Check if either endpoint is inside the rectangle
        if ((rectMin.x <= start.x && start.x <= rectMax.x && rectMin.y <= start.y && start.y <= rectMax.y) ||
            (rectMin.x <= end.x && end.x <= rectMax.x && rectMin.y <= end.y && end.y <= rectMax.y))
        {
            return true;
        }

        float dx = end.x - start.x;
        float dy = end.y - start.y;

        // Check for intersection with each edge of the rectangle
        if (dx != 0) {
            float t1 = (rectMin.x - start.x) / dx;
            float t2 = (rectMax.x - start.x) / dx;
            if (0 <= t1 && t1 <= 1) {
                float y = start.y + t1 * dy;
                if (rectMin.y <= y && y <= rectMax.y) return true;
            }
            if (0 <= t2 && t2 <= 1) {
                float y = start.y + t2 * dy;
                if (rectMin.y <= y && y <= rectMax.y) return true;
            }
        }
        if (dy != 0) {
            float t1 = (rectMin.y - start.y) / dy;
            float t2 = (rectMax.y - start.y) / dy;
            if (0 <= t1 && t1 <= 1) {
                float x = start.x + t1 * dx;
                if (rectMin.x <= x && x <= rectMax.x) return true;
            }
            if (0 <= t2 && t2 <= 1) {
                float x = start.x + t2 * dx;
                if (rectMin.x <= x && x <= rectMax.x) return true;
            }
        }

        return false;
    }
}

void LinkManager::UpdateLinkSelection(const Vec2f& origin, float zoom)
{
    UserInputState userInputState = m_nodeManager->GetUserInputState();
    if (userInputState == UserInputState::None && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (auto clickedLink = GetLinkClicked(zoom, origin, ImGui::GetMousePos()).lock())
        {
            AddSelectedLink(clickedLink);
        }
    }
    
    SelectionSquare selectionSquare = m_nodeManager->GetSelectionSquare();

    if (!selectionSquare.shouldDraw)
        return;

    ClearSelectedLinks();

    for (const LinkRef& link : m_links)
    {
        // check if the link is inside the selection square
        Vec2f positionIn = m_nodeManager->GetNode(link->fromNodeIndex).lock()->GetOutputPosition(link->fromOutputIndex, origin, zoom);
        Vec2f positionOut = m_nodeManager->GetNode(link->toNodeIndex).lock()->GetInputPosition(link->toInputIndex, origin, zoom);
        if (BezierIntersectSquare(positionIn, positionIn + Vec2f(m_controlDistanceX, 0.0f) * zoom,
                                  positionOut - Vec2f(m_controlDistanceX, 0.0f) * zoom, positionOut,
                                  selectionSquare.min, selectionSquare.max))
        {
            m_selectedLinks.push_back(link);
        }
    }
}

void LinkManager::UpdateInputOutputLinks()
{
    for (int i = 0; i < m_links.size(); i++)
    {
        auto& link = m_links[i];
        if (!m_nodeManager->NodeExists(link->toNodeIndex) || !m_nodeManager->NodeExists(link->fromNodeIndex))
        {
            RemoveLink(i--, false);
            continue;
        }
        InputRef input = m_nodeManager->GetInput(link->toNodeIndex, link->toInputIndex).lock();
        OutputRef output = m_nodeManager->GetOutput(link->fromNodeIndex, link->fromOutputIndex).lock();
        if (input)
            input->SetLinked(output != nullptr);
        if (output)
            output->SetLinked(input != nullptr);
    }
}

void LinkManager::DrawLinks(float zoom, const Vec2f& origin)
{
    auto drawList = ImGui::GetWindowDrawList();

    for (uint32_t i = 0; i < m_selectedLinks.size(); i++)
    {
        LinkRef selectedLink = m_selectedLinks[i].lock();
        NodeRef fromNode = m_nodeManager->GetNode(selectedLink->fromNodeIndex).lock();
        NodeRef toNode = m_nodeManager->GetNode(selectedLink->toNodeIndex).lock();

        if (!fromNode || !toNode)
        {
            m_selectedLinks.erase(m_selectedLinks.begin() + i);
            continue;
        }
        
        assert(toNode != nullptr && fromNode != nullptr && "Node not found");
        
        Vec2f inputPosition = fromNode->GetOutputPosition(selectedLink->fromOutputIndex, origin, zoom);
        Vec2f outputPosition = toNode->GetInputPosition(selectedLink->toInputIndex, origin, zoom);
        
        Vec2f controlPoint1 = inputPosition + Vec2f(m_controlDistanceX, 0.0f) * zoom;
        Vec2f controlPoint2 = outputPosition - Vec2f(m_controlDistanceX, 0.0f) * zoom;
        
        drawList->AddBezierCubic(inputPosition, controlPoint1, controlPoint2, outputPosition, IM_COL32(255, 255, 0, 255), 3 * zoom, m_bezierSegmentCount);
    }
    
    for (uint32_t i = 0; i < m_links.size(); i++)
    {
        LinkRef link = m_links[i];
        NodeRef fromNode = m_nodeManager->GetNode(link->fromNodeIndex).lock();
        NodeRef toNode = m_nodeManager->GetNode(link->toNodeIndex).lock();
        
        if (!fromNode || !toNode)
        {
            RemoveLink(i);
            i--;
            continue;
        }
        
        assert(toNode != nullptr && fromNode != nullptr && "Node not found");
        
        Vec2f inputPosition = fromNode->GetOutputPosition(link->fromOutputIndex, origin, zoom);
        Vec2f outputPosition = toNode->GetInputPosition(link->toInputIndex, origin, zoom);
        
        Vec2f controlPoint1 = inputPosition + Vec2f(m_controlDistanceX, 0.0f) * zoom;
        Vec2f controlPoint2 = outputPosition - Vec2f(m_controlDistanceX, 0.0f) * zoom;

        drawList->AddCircleFilled(inputPosition, 4.f * zoom, IM_COL32(200, 200, 200, 255));
        drawList->AddCircleFilled(outputPosition, 4.f * zoom, IM_COL32(200, 200, 200, 255));
        
        drawList->AddBezierCubic(inputPosition, controlPoint1, controlPoint2, outputPosition, IM_COL32(255, 255, 255, 255), 2 * zoom, m_bezierSegmentCount);
    }
}

void LinkManager::CreateLink(const NodeRef& fromNode, const uint32_t fromOutput, const NodeRef& toNode, const uint32_t toOutput)
{
    CreateLink(fromNode->GetUUID(), fromOutput, toNode->GetUUID(), toOutput);
}

void LinkManager::CreateLink(UUID fromNodeIndex, const uint32_t fromOutputIndex, UUID toNodeIndex, const uint32_t toOutputIndex)
{
    AddLink(std::make_shared<Link>(std::move(fromNodeIndex), fromOutputIndex, std::move(toNodeIndex), toOutputIndex));
}

LinkWeakRef LinkManager::AddLink(Link link)
{
    auto sharedPtr = std::make_shared<Link>(std::move(link));
    AddLink(sharedPtr);
    return sharedPtr;
}

void LinkManager::AddLink(const LinkRef& link)
{
    auto input = m_nodeManager->GetInput(link->toNodeIndex, link->toInputIndex);
    auto output = m_nodeManager->GetOutput(link->fromNodeIndex, link->fromOutputIndex);

    input.lock()->SetLinked(true);
    output.lock()->SetLinked(true);
    
    m_links.push_back(link);
}

void LinkManager::RemoveLink(uint32_t index, bool removeOnLink /*= true*/)
{
    if (removeOnLink)
    {
        auto input = m_nodeManager->GetInput(m_links[index]->toNodeIndex, m_links[index]->toInputIndex);
        auto output = m_nodeManager->GetOutput(m_links[index]->fromNodeIndex, m_links[index]->fromOutputIndex);

        input.lock()->SetLinked(false);
        output.lock()->SetLinked(false);
    }
    m_links.erase(m_links.begin() + index);
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
            RemoveLink(i);
            break;
        }
    }
}

void LinkManager::RemoveLink(const LinkWeakRef& link)
{
    LinkRef sharedPtr = link.lock();
    RemoveLink(sharedPtr->fromNodeIndex, sharedPtr->fromOutputIndex, sharedPtr->toNodeIndex, sharedPtr->toInputIndex);
}

void LinkManager::RemoveLink(const InputRef& input)
{
    for (uint32_t i = 0; i < m_links.size(); i++)
    {
        if (m_links[i]->toNodeIndex == input->parentUUID && m_links[i]->toInputIndex == input->index)
        {
            RemoveLink(i);
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
            RemoveLink(i);
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
            RemoveLink(i);
            i--;
        }
    }
}

bool LinkManager::CanCreateLink(const Link& link) const
{
    if (link.fromNodeIndex == UUID_NULL || link.toNodeIndex == UUID_NULL)
        return false;    
    NodeRef fromNode = m_nodeManager->GetNode(link.fromNodeIndex).lock();
    if (!fromNode)
        return false;
    OutputRef fromOutput = fromNode->GetOutput(link.fromOutputIndex);

    
    NodeRef toNode = m_nodeManager->GetNode(link.toNodeIndex).lock();
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

bool LinkManager::BezierIntersectSquare(Vec2f inputPosition, Vec2f controlPoint1, Vec2f controlPoint2,
    Vec2f outputPosition, Vec2f rectMin, Vec2f rectMax)
{
    const int segments = 10; // Increase for higher accuracy
    std::vector<Vec2f> bezierPoints;

    // Generate points along the Bezier curve
    for (int i = 0; i <= segments; ++i)
    {
        float t = static_cast<float>(i) / segments;
        bezierPoints.push_back(Utils::CubicBezierPoint(inputPosition, controlPoint1, controlPoint2, outputPosition, t));
    }

    // Check each segment of the Bezier curve against the rectangle
    for (int i = 0; i < segments; ++i)
    {
        if (Utils::LineIntersectsRect(bezierPoints[i], bezierPoints[i + 1], rectMin, rectMax))
        {
            return true;
        }
    }

    return false;
}

std::vector<LinkWeakRef> LinkManager::GetLinkWithOutput(const OutputRef& output) const
{
    std::vector<LinkWeakRef> links;
    for (uint32_t i = 0; i < m_links.size(); i++)
    {
        if (m_links[i]->fromNodeIndex == output->parentUUID && m_links[i]->fromOutputIndex == output->index)
        {
            links.push_back(m_links[i]);
        }
    }
    return links;
}

LinkWeakRef LinkManager::GetLinkLinkedToInput(const UUID& uuid, uint32_t index) const
{
    for (const LinkRef& link : m_links)
    {
        if (link->toNodeIndex == uuid && link->toInputIndex == index)
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
    return std::any_of(m_links.begin(), m_links.end(), [&](const LinkRef& link) {
        return link->toNodeIndex == input->parentUUID && link->toInputIndex == input->index;
    });
}

LinkWeakRef LinkManager::GetLinkClicked(float zoom, const Vec2f& origin, const Vec2f& mousePos) const
{
    for (const LinkRef& link : m_links)
    {
        NodeRef fromNode = m_nodeManager->GetNode(link->fromNodeIndex).lock();
        NodeRef toNode = m_nodeManager->GetNode(link->toNodeIndex).lock();
        
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

void LinkManager::AddSelectedLink(const LinkRef& link)
{
    if (std::ranges::find_if(m_selectedLinks, [link](const LinkWeakRef& l) { return l.lock() == link; }) == m_selectedLinks.end())
        m_selectedLinks.push_back(link);
}

void LinkManager::DeleteSelectedLinks()
{
    for (const auto& i : m_selectedLinks)
    {
        RemoveLink(i);
    }
    m_selectedLinks.clear();
}

void LinkManager::ClearSelectedLinks()
{
    m_selectedLinks.clear();
}

void LinkManager::Serialize(CppSer::Serializer& serializer) const
{
    Serialize(serializer, m_links);
}

void LinkManager::Serialize(CppSer::Serializer& serializer, const std::vector<LinkRef>& links)
{
    serializer << CppSer::Pair::BeginMap << "Links";
    serializer << CppSer::Pair::Key << "Link Count" << CppSer::Pair::Value << links.size();
    serializer << CppSer::Pair::BeginTab;
    for (const LinkRef& link : links)
    {
        serializer << CppSer::Pair::BeginMap << "Link";
        serializer << CppSer::Pair::Key << "From Node Index" << CppSer::Pair::Value << link->fromNodeIndex;
        serializer << CppSer::Pair::Key << "From Output Index" << CppSer::Pair::Value << link->fromOutputIndex;
        serializer << CppSer::Pair::Key << "To Node Index" << CppSer::Pair::Value << link->toNodeIndex;
        serializer << CppSer::Pair::Key << "To Input Index" << CppSer::Pair::Value << link->toInputIndex;
        serializer << CppSer::Pair::EndMap << "Link";
    }
    serializer << CppSer::Pair::EndTab;
    serializer << CppSer::Pair::EndMap << "Links";
}

void LinkManager::Deserialize(CppSer::Parser& parser)
{
    Deserialize(parser, m_links);
    UpdateInputOutputLinks();
}

void LinkManager::Deserialize(CppSer::Parser& parser, std::vector<LinkRef>& links)
{
    parser.PushDepth();
    uint32_t linkCount = parser["Link Count"].As<uint32_t>();
    links.resize(linkCount);
    for (uint32_t i = 0; i < linkCount; i++)
    {
        parser.PushDepth();
        LinkRef link = std::make_shared<Link>();
        link->fromNodeIndex = parser["From Node Index"].As<uint64_t>();
        link->fromOutputIndex = parser["From Output Index"].As<uint32_t>();
        link->toNodeIndex = parser["To Node Index"].As<uint64_t>();
        link->toInputIndex = parser["To Input Index"].As<uint32_t>();

        links[i] = link;
    }
}

void LinkManager::Clean()
{
    for (int i = 0; i < m_links.size(); i++)
    {
        RemoveLink(i--);
    }
    
    m_selectedLinks.clear();
}

MainWindow* LinkManager::GetMainWindow() const
{
    return m_nodeManager->GetMainWindow();
}
