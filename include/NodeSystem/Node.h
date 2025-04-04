#pragma once
#include <iostream>
#include <vector>
#include <galaxymath/Maths.h>
#include <optional>
#include <filesystem>

#include "NodeSystem/Selectable.h"

#include "UUID.h"
#include "Type.h"

class Mesh;
class Framebuffer;
class Shader;
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
    Vector4,
    Sampler2D,
};

enum class StreamType
{
    None,
    Input,
    Output,
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
        possibleTypes.push_back(type);
    }
    virtual ~Stream() = default;

    void SetLinked(bool value) { isLinked = value; }

    bool HasType(Type type) const
    {
        return std::ranges::find(possibleTypes, type) != possibleTypes.end();
    }

    UUID parentUUID = -1;
    uint32_t index = -1;
    
    std::string name;
    Type type = Type::None;

    std::vector<Type> possibleTypes;
    
    bool isLinked = false;

    bool isHovered = false;

    StreamType streamType = StreamType::None;
};

struct Input : Stream
{
    Input() = default;
    Input(const UUID& _parentUUID, uint32_t _index, const std::string& _name, Type _type);
    
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
    Output(const UUID& _parentUUID, uint32_t _index, const std::string& _name, Type _type);
};

constexpr int c_pointSize = 25;
constexpr int c_topSize = 25;

typedef Ref<Stream> StreamRef;
typedef Weak<Stream> StreamWeak;

typedef Ref<Input> InputRef;
typedef Weak<Input> InputWeak;

typedef Ref<Output> OutputRef;
typedef Weak<Output> OutputWeak;

using LinkWeakRef = std::weak_ptr<struct Link>;
using LinkRef = std::shared_ptr<struct Link>;

uint32_t GetColor(Type type);

static void AddColor(uint32_t& color, int value);
constexpr float c_streamCircleRadius = 5.0f;
constexpr float c_hoveredCircleRadiusFactor = 1.2f;

static void DrawTriangle(ImDrawList* draw_list, ImGuiDir dir, const Vec2f& position, const Vec2f& size);

class Node : public Selectable
{
public:
    explicit Node();
    Node(std::string _name);
    ~Node() override;

    // Draw Methods
    void Draw(float zoom, const Vec2f& origin) const;
    void DrawDot(float zoom, const Vec2f& origin, uint32_t i, bool isOutput) const;
    void DrawOutputDot(float zoom, const Vec2f& origin, uint32_t i) const;
    void DrawInputDot(float zoom, const Vec2f& origin, uint32_t i) const;
    void DrawButtonPreview(float zoom, const Vec2f& origin, Vec2f pMin, Vec2f pMax) const;

    void RenderPreview(std::shared_ptr<Mesh> quad) const;

    virtual void Update();
    void GetPreviewRect(const Vec2f& pMin, float zoom, Vec2f& imageMin, Vec2f& imageMax) const;
    void DrawPreview(Vec2f pMin, float zoom) const;
    
    int FindBestPossibilityForType(Type type, StreamRef stream) const;
    
    void ConvertStream(uint32_t index);

    static bool IsPointHoverCircle(const Vec2f& point, const Vec2f& circlePos, const Vec2f& origin, float zoom, uint32_t index);

    // Selected Methods
    bool IsSelected(const Vec2f& point, const Vec2f& origin, float zoom) const override { return IsPointHoverNode(point, origin, zoom); }
    bool IsSelected(const Vec2f& rectMin, const Vec2f& rectMax, const Vec2f& origin, float zoom) const override;
    bool IsPreviewHovered(const Vec2f& point, const Vec2f& origin, float zoom) const;

    Vec2f GetMin(float zoom, const Vec2f& origin) const 
    {
        return origin + p_position * zoom;
    }
    Vec2f GetMax(const Vec2f& min, float zoom) const
    {
        return min + (p_preview ? p_sizeWithPreview : p_size) * zoom;
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

    std::vector<std::vector<Type>> GetAllPossibilities() const;

    void ChangeInputType(uint32_t index, Type type) const;
    void ChangeOutputType(uint32_t index, Type type) const;

    void RemoveInput(uint32_t index);
    void RemoveOutput(uint32_t index);

    virtual void ClearInputs();
    virtual void ClearOutputs();

    InputRef GetInput(uint32_t index) { return p_inputs.size() > index ? p_inputs[index] : nullptr; }
    OutputRef GetOutput(uint32_t index) { return p_outputs.size() > index ? p_outputs[index] : nullptr; }

    Vec2f GetInputPosition(uint32_t index, const Vec2f& origin, float zoom) const;
    Vec2f GetInputPosition(uint32_t index) const;
    Vec2f GetOutputPosition(uint32_t index, const Vec2f& origin, float zoom) const;
    Vec2f GetOutputPosition(uint32_t index) const;

    std::vector<LinkWeakRef> GetLinks() const;

    void SetPosition(const Vec2f& position);
    void SetName(std::string name) { p_name = std::move(name); }
    void SetTopColor(uint32_t color) { p_topColor = color; }
    void ResetUUID();
    void ComputeNodeSize();

    UUID GetUUID() const { return p_uuid; }
    std::string GetName() const { return p_name; }
    Vec2f GetPosition() const { return p_position; }
    Vec2f GetSize() const { return p_size; }
    TemplateID GetTemplateID() const { return p_templateID; }
    bool GetAllowInteraction() const { return p_allowInteraction; }
    NodeManager* GetNodeManager() const { return p_nodeManager; }
    std::vector<InputRef>& GetInputs() { return p_inputs; }
    std::vector<OutputRef>& GetOutputs() { return p_outputs; }

    static void GetPreviewTriangle(Vec2f& trianglePos, Vec2f& triangleSize, const Vec2f& nodeMin, const Vec2f& nodeMax, float zoom);
    static void GetPreviewButtonRect(Vec2f& outMin, Vec2f& outMax, const Vec2f& nodeMin, const Vec2f& nodeMax, float zoom);
    
    virtual void ShowInInspector();

    virtual void ShowInputInspector(uint32_t index);

    const std::filesystem::path& GetTexturePath() const { return m_texturePath.value_or(""); }
    void SetTexturePath(const std::filesystem::path& path) { if (path.empty()) {m_texturePath = path;} }

    virtual std::vector<std::string> GetFormatStrings() const;

    virtual void Serialize(CppSer::Serializer& serializer) const;
    virtual void InternalSerialize(CppSer::Serializer& serializer) const;
    virtual void Deserialize(CppSer::Parser& parser);
    virtual void InternalDeserialize(CppSer::Parser& parser);

    virtual std::string ToShader(ShaderMaker* shaderMaker, const FuncStruct& funcStruct) const;

    virtual Node* Clone() const;

    virtual void OnChangeUUID(const UUID& prevUUID, const UUID& newUUID);

    virtual void InitializePreview();
    void OpenPreview(bool open);

    void RecalculateWidth();
protected:
    void SetUUID(const UUID& uuid);

    void Internal_Clone(Node* node) const;

    // Called when the node is created
    virtual void OnCreate() {}
    // Called when the node is removed from the node manager
    virtual void OnRemove() {}


protected:
    friend class NodeWindow;
    friend class ShaderMaker;
    friend class NodeTemplateHandler;
    friend class NodeManager;
    
    UUID p_uuid;
    std::string p_name;
    
    std::vector<InputRef> p_inputs;
    std::vector<OutputRef> p_outputs;

    int p_currentPossibility = 0;
    int p_possiblityCount = 0;
    
    Vec2f p_position;
    Vec2f p_size = {125.0f, c_topSize};
    Vec2f p_sizeWithPreview;
    
    TemplateID p_templateID = -1;
    uint32_t p_topColor = IM_COL32(150, 150, 150, 255);
    
    NodeManager* p_nodeManager;

    bool p_loaded = false;
    
    bool p_selected = false;
    Vec2f p_positionOnClick = {0, 0};

    bool p_allowInteraction = true; // Used for nodes that are not supposed to be delted or copied
    bool p_alwaysVisibleOnContext = false;
    bool p_allowPreview = true;
    bool p_isVisible = true;
    bool p_computed = false;

    bool p_previewHovered = false;
    bool p_preview = false;
    Ref<Shader> m_shader;
    Ref<Framebuffer> m_framebuffer;
    
    std::optional<std::filesystem::path> m_texturePath;
};

typedef std::shared_ptr<Node> NodeRef;
typedef std::weak_ptr<Node> NodeWeak;