#pragma once
#include <iostream>
#include <vector>

#include <galaxymath/maths.h>

#include "Selectable.h"
#include "UUID.h"
using namespace GALAXY;
#include <imgui.h>

namespace CppSer { class Serializer; class Parser; }

enum class Type
{
    Float,
    Int,
    Bool,
    String,
    Vector2,
    Vector3,
};

std::string TypeEnumToString(Type type);

struct Input
{
    Input(Type type);
    Input(const UUID& _parentUUID, uint32_t _index, const std::string& _name, Type _type);
    ~Input();
    
    UUID parentUUID;
    uint32_t index;
    
    std::string name;
    Type type;
    
    void* value = nullptr;
    bool isLinked = false;

    template<typename T>
    T* GetValue() const
    {
        return reinterpret_cast<T*>(value);
    }

    template<typename T>
    void SetValue(T _value)
    {
        if (value != nullptr)
            delete value;
        this->value = new T(_value);
    }

    template <typename T>
    Input* Clone()
    {
        Input* input = new Input(parentUUID, index, name, type);
        input->isLinked = isLinked;
        input->value = new T(*GetValue<T>());
        return input;
    }

};

struct Output
{
    UUID parentUUID;
    uint32_t index;
    
    std::string name;
    Type type;
    
    bool isLinked;
};

constexpr int c_pointSize = 25;
constexpr int c_topSize = 25;

typedef std::shared_ptr<Input> InputRef;
typedef std::weak_ptr<Input> InputWeakRef;

typedef std::shared_ptr<Output> OutputRef;
typedef std::weak_ptr<Output> OutputWeakRef;

uint32_t GetColor(Type type);

class Node : public Selectable
{
public:
    explicit Node();
    Node(std::string _name);
    ~Node() = default;

    // Draw Methods
    void Draw(float zoom, const Vec2f& origin) const;
    void DrawOutputDot(float zoom, const Vec2f& origin, uint32_t i) const;
    void DrawInputDot(float zoom, const Vec2f& origin, uint32_t i) const;

    static bool IsPointHoverCircle(const Vec2f& point, const Vec2f& circlePos, const Vec2f& origin, float zoom, uint32_t index);

    // Selected Methods
    bool IsSelected(const Vec2f& point, const Vec2f& origin, float zoom) const override { return IsPointHoverNode(point, origin, zoom); }
    bool IsSelected(const Vec2f& rectMin, const Vec2f& rectMax, const Vec2f& origin, float zoom) const override;

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

    bool IsNodeVisible(const Vec2f& origin, float zoom) const;

    bool IsInputClicked(const Vec2f& point, const Vec2f& origin, float zoom, uint32_t& index) const;
    bool IsOutputClicked(const Vec2f& point, const Vec2f& origin, float zoom, uint32_t& index) const;
    bool DoesOutputHaveLink(uint32_t index) const;
    bool DoesInputHaveLink(uint32_t index) const;

    void AddInput(const std::string& name, Type type);
    void AddOutput(const std::string& name, Type type);

    InputRef GetInput(uint32_t index) { return p_inputs[index]; }
    OutputRef GetOutput(uint32_t index) { return p_outputs[index]; }

    Vec2f GetInputPosition(uint32_t index, const Vec2f& origin, float zoom) const;
    Vec2f GetOutputPosition(uint32_t index, const Vec2f& origin, float zoom) const;

    using LinkWeakRef = std::weak_ptr<struct Link>;
    std::vector<LinkWeakRef> GetLinks() const;
    LinkWeakRef GetLinkWithOutput(uint32_t index) const;
    std::vector<LinkWeakRef> GetLinksWithInput(uint32_t index) const;

    void SetPosition(const Vec2f& position);
    void SetName(std::string name) { p_name = std::move(name); }
    void SetTopColor(uint32_t color) { p_topColor = color; }
    void ResetUUID();

    UUID GetUUID() const { return p_uuid; }
    std::string GetName() const { return p_name; }
    Vec2f GetPosition() const { return p_position; }
    Vec2f GetSize() const { return p_size; }
    TemplateID GetTemplateID() const { return p_templateID; }
    bool GetAllowInteraction() const { return p_allowInteraction; }

    virtual void Serialize(CppSer::Serializer& serializer) const;
    virtual void Deserialize(CppSer::Parser& parser);

    virtual Node* Clone();
private:
    void SetUUID(const UUID& uuid);

protected:
    friend class MainWindow;
    friend class ShaderMaker;
    friend class NodeTemplateHandler;
    friend class NodeManager;

    TemplateID p_templateID = -1;

    NodeManager* p_nodeManager;

    uint32_t p_topColor = IM_COL32(150, 150, 150, 255);

    UUID p_uuid;
    std::string p_name;
    
    std::vector<InputRef> p_inputs;
    std::vector<OutputRef> p_outputs;
    
    Vec2f p_position;
    Vec2f p_size = {150.0f, c_topSize};
    
    bool p_selected = false;
    Vec2f p_positionOnClick = {0, 0};

    bool p_allowInteraction = true; // Used for nodes that are not supposed to be delted or copied
    bool p_isVisible = true;
};

typedef std::shared_ptr<Node> NodeRef;
typedef std::weak_ptr<Node> NodeWeakRef;