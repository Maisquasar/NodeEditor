#pragma once
#include <iostream>
#include <vector>

#include <galaxymath/maths.h>

#include "NodeSystem/Selectable.h"
#include "UUID.h"

#include "Type.h"

class NodeManager;
class ShaderMaker;
struct FuncStruct;
using namespace GALAXY;
#include <imgui.h>

namespace CppSer { class Serializer; class Parser; }

enum class Type
{
    None = 0,
    Float,
    Int,
    Bool,
    Vector2,
    Vector3,
    Vector4
};

const char* SerializeTypeEnum();
std::string TypeEnumToString(Type type);
uint32_t GetColorFromType(Type type);

struct Stream
{
    Stream() = default;
    Stream(const UUID& _parentUUID, const uint32_t _index, const std::string& _name, const Type _type) :
        parentUUID(_parentUUID), index(_index), name(_name), type(_type), isLinked(false)
    {
    }
    virtual ~Stream() = default;

    void SetLinked(bool value) { isLinked = value; }

    UUID parentUUID = -1;
    uint32_t index = -1;
    
    std::string name;
    Type type = Type::None;
    
    bool isLinked = false;
};

struct Input : Stream
{
    Input(Type type);
    Input(const UUID& _parentUUID, uint32_t _index, const std::string& _name, Type _type);
    ~Input() = default;
    
    Vec4f value;

    template<typename T>
    T GetValue() const;

    Vec4f GetValue() const { return value; }

    template<typename T>
    void SetValue(const T& _value);

    Input* Clone() const;
};


struct Output : Stream
{
    Output() = default;
    Output(const UUID& _parentUUID, uint32_t _index, const std::string& _name, Type _type) : Stream(_parentUUID, _index, _name, _type) {}
};

constexpr int c_pointSize = 25;
constexpr int c_topSize = 25;

typedef Ref<Stream> StreamRef;
typedef Weak<Stream> StreamWeak;

typedef Ref<Input> InputRef;
typedef Weak<Input> InputWeak;

typedef Ref<Output> OutputRef;
typedef Weak<Output> OutputWeak;

uint32_t GetColor(Type type);

constexpr float streamCircleRadius = 5.0f;

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
    static int CalculateSize(int size);

    void AddInput(const std::string& name, Type type);
    void AddOutput(const std::string& name, Type type);

    void RemoveInput(uint32_t index);
    void RemoveOutput(uint32_t index);

    InputRef GetInput(uint32_t index) { return p_inputs[index]; }
    OutputRef GetOutput(uint32_t index) { return p_outputs[index]; }

    Vec2f GetInputPosition(uint32_t index, const Vec2f& origin, float zoom) const;
    Vec2f GetOutputPosition(uint32_t index, const Vec2f& origin, float zoom) const;

    using LinkWeakRef = std::weak_ptr<struct Link>;
    std::vector<LinkWeakRef> GetLinks() const;

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
    NodeManager* GetNodeManager() const { return p_nodeManager; }
    
    virtual void ShowInInspector();

    virtual std::vector<std::string> GetFormatStrings() const;

    virtual void Serialize(CppSer::Serializer& serializer) const;
    virtual void InternalSerialize(CppSer::Serializer& serializer) const;
    virtual void Deserialize(CppSer::Parser& parser);
    virtual void InternalDeserialize(CppSer::Parser& parser);

    virtual std::string ToShader(ShaderMaker* shaderMaker, const FuncStruct& funcStruct) const;

    virtual Node* Clone() const;
protected:
    void SetUUID(const UUID& uuid);

    void Internal_Clone(Node* node) const;

protected:
    friend class NodeWindow;
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
    bool p_alwaysVisibleOnContext = false;
    bool p_isVisible = true;
};

typedef std::shared_ptr<Node> NodeRef;
typedef std::weak_ptr<Node> NodeWeak;