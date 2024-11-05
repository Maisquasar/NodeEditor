#pragma once
#include <cstdint>
#include <vector>

#include <Maths.h>

#include "Node.h"
#include "UUID.h"

struct Link
{
    UUID fromNodeIndex = -1;
    uint32_t fromOutputIndex = -1;

    UUID toNodeIndex = -1;
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

    static bool IsPointHoverLineSegment(Vec2f pointPosition, Vec2f fromPosition, Vec2f toPosition, float threshold = 1);

    LinkWeakRef GetLinkWithOutput(const UUID& uuid, uint32_t index) const;
    LinkWeakRef GetLinkClicked(float zoom, const Vec2f& origin, const Vec2f& mousePos) const;
private:
    
    std::vector<LinkRef> m_links;
    
};
