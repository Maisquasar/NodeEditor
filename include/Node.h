#pragma once
#include <iostream>
#include <vector>

#include <galaxymath/maths.h>

#include "UUID.h"
using namespace GALAXY;
#include <imgui.h>


enum class Type
{
    Float,
    Int,
    Bool,
    String,
    Vector2,
    Vector3,
};

struct Input
{
    UUID parentUUID;
    uint32_t index;
    
    std::string name;
    Type type;
};

struct Output
{
    UUID parentUUID;
    uint32_t index;
    
    std::string name;
    Type type;
};

constexpr int c_pointSize = 25;
constexpr int c_topSize = 25;

typedef std::shared_ptr<Input> InputRef;
typedef std::weak_ptr<Input> InputWeakRef;

typedef std::shared_ptr<Output> OutputRef;
typedef std::weak_ptr<Output> OutputWeakRef;

uint32_t GetColor(Type type);

class Node
{
public:
    Node(std::string _name);
    ~Node() = default;
    
    void Draw(float zoom, const Vec2f& origin) const;

    static bool IsPointHoverCircle(const Vec2f& point, const Vec2f& circlePos, const Vec2f& origin, float zoom, uint32_t index);

    Vec2f GetMin(float zoom, const Vec2f& origin) const 
    {
        return origin + p_position * zoom;
    }
    Vec2f GetMax(const Vec2f& min, float zoom) const
    {
        return min + p_size * zoom;
    }
    Vec2f GetMax(float zoom, const Vec2f& origin) const
    {
        return GetMin(zoom, origin) + p_size * zoom;
    }

    bool IsPointHoverNode(const Vec2f& point, const Vec2f& origin, float zoom) const
    {
        Vec2f min = GetMin(zoom, origin);
        Vec2f max = GetMax(min, zoom);
        return point.x > min.x && point.x < max.x && point.y > min.y && point.y < max.y;
    }

    bool IsInputClicked(const Vec2f& point, const Vec2f& origin, float zoom, int& index) const;
    bool IsOutputClicked(const Vec2f& point, const Vec2f& origin, float zoom, int& index) const;
    bool DoesOutputHaveLink(uint32_t index) const;

    void AddInput(std::string name, Type type);
    void AddOutput(std::string name, Type type);

    InputRef GetInput(uint32_t index) { return p_inputs[index]; }
    OutputRef GetOutput(uint32_t index) { return p_outputs[index]; }

    Vec2f GetInputPosition(uint32_t index) const;
    Vec2f GetInputPosition(uint32_t index, const Vec2f& origin, float zoom) const;
    Vec2f GetOutputPosition(uint32_t index) const;
    Vec2f GetOutputPosition(uint32_t index, const Vec2f& origin, float zoom) const;

    std::weak_ptr<struct Link> GetLinkWithOutput(uint32_t index) const;

    void SetPosition(Vec2f position);
    void SetName(std::string name) { p_name = std::move(name); }
    void SetTopColor(uint32_t color) { p_topColor = color; }
    
    UUID GetUUID() const { return p_uuid; }

protected:
    friend class NodeManager;

    NodeManager* p_nodeManager;

    uint32_t p_topColor = IM_COL32(150, 150, 150, 255);

    UUID p_uuid;
    std::string p_name;
    
    std::vector<InputRef> p_inputs;
    std::vector<OutputRef> p_outputs;
    
    Vec2f p_position;
    Vec2f p_size = {150.0f, c_topSize};
    
    bool p_selected = false;
};

typedef std::shared_ptr<Node> NodeRef;
typedef std::weak_ptr<Node> NodeWeakRef;