#pragma once
#include <cstdint>
#include <vector>

#include <Maths.h>

#include "Node.h"
#include "UUID.h"

struct Link
{    
    UUID fromNodeIndex = UUID_NULL;
    uint32_t fromOutputIndex = -1;

    UUID toNodeIndex = UUID_NULL;
    uint32_t toInputIndex = -1;
};

using LinkRef = std::shared_ptr<Link>;
using LinkWeakRef = std::weak_ptr<Link>;

class LinkManager
{
public:
    LinkManager(NodeManager* nodeManager) : m_nodeManager(nodeManager) {}

    void UpdateLinkSelection(const Vec2f& origin, float zoom);
    void UpdateInputOutputLinks();
    void DrawLinks(float zoom, const Vec2f& origin);

    void CreateLink(const NodeRef& fromNode, uint32_t fromOutput, const NodeRef& toNode, uint32_t toOutput);
    void CreateLink(UUID fromNodeIndex, uint32_t fromOutputIndex, UUID toNodeIndex, uint32_t toOutputIndex);

    LinkWeakRef AddLink(Link link);
    void AddLink(const LinkRef& link);

    void RemoveLink(uint32_t index, bool removeOnLink = true);
    void RemoveLink(const NodeRef& fromNode, uint32_t fromOutput, const NodeRef& toNode, uint32_t toOutput);
    void RemoveLink(const UUID& fromNodeIndex, uint32_t fromOutputIndex, const UUID& toNodeIndex, uint32_t toOutputIndex);
    void RemoveLink(const LinkWeakRef& link);
    // Removes the link connected to the input
    void RemoveLink(const InputRef& input);
    // Removes all links connected to the output
    void RemoveLinks(const OutputRef& output);
    
    void RemoveLinks(const NodeRef& node);

    bool CanCreateLink(const Link& link) const;
    


    static bool IsPointHoverLineSegment(Vec2f pointPosition, Vec2f fromPosition, Vec2f toPosition, float threshold = 1);
    static bool IsPointHoverBezier(Vec2f pointPosition, Vec2f inputPosition, Vec2f controlPoint1, Vec2f controlPoint2, Vec2f outputPosition, float threshold = 1, int numSamples = 10);
    static bool BezierIntersectSquare(Vec2f inputPosition, Vec2f controlPoint1, Vec2f controlPoint2, Vec2f outputPosition, Vec2f rectMin, Vec2f rectMax);

    std::vector<LinkWeakRef> GetLinkWithOutput(const OutputRef& output) const;
    LinkWeakRef GetLinkLinkedToInput(const UUID& uuid, uint32_t index) const;
    std::vector<LinkWeakRef> GetLinksWithInput(const UUID& uuid, uint32_t index) const;
    const std::vector<LinkRef>& GetLinks() const { return m_links; }
    const std::vector<LinkWeakRef>& GetSelectedLinks() { return m_selectedLinks;}

    bool HasLink(const OutputRef& output) const;
    bool HasLink(const InputRef& input) const;
    
    LinkWeakRef GetLinkClicked(float zoom, const Vec2f& origin, const Vec2f& mousePos) const;

    void AddSelectedLink(const LinkRef& link);

    void DeleteSelectedLinks();
    void ClearSelectedLinks();
    
    void Serialize(CppSer::Serializer& serializer) const;
    static void Serialize(CppSer::Serializer& serializer, const std::vector<LinkRef>& links);
    void Deserialize(CppSer::Parser& parser);
    static void Deserialize(CppSer::Parser& parser, std::vector<LinkRef>& links);

    void Clean();
private:
    NodeManager* m_nodeManager = nullptr;
    
    std::vector<LinkRef> m_links;
    std::vector<LinkWeakRef> m_selectedLinks;
    
    float m_controlDistanceX = 50.f;
    int m_bezierSegmentCount = 25;
};
