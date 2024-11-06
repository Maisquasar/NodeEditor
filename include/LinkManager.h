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
    
    void DrawLinks(float zoom, const Vec2f& origin) const;

    void CreateLink(const NodeRef& fromNode, uint32_t fromOutput, const NodeRef& toNode, uint32_t toOutput);
    void CreateLink(UUID fromNodeIndex, uint32_t fromOutputIndex, UUID toNodeIndex, uint32_t toOutputIndex);

    void AddLink(Link link);

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

    LinkWeakRef GetLinkWithOutput(const UUID& uuid, uint32_t index) const;
    std::vector<LinkWeakRef> GetLinksWithInput(const UUID& uuid, uint32_t index) const;

    bool HasLink(const OutputRef& output) const;
    bool HasLink(const InputRef& input) const;
    
    LinkWeakRef GetLinkClicked(float zoom, const Vec2f& origin, const Vec2f& mousePos) const;
private:
    
    std::vector<LinkRef> m_links;
    float m_controlDistanceX = 50.f;
    int m_bezierSegmentCount = 10;
};
