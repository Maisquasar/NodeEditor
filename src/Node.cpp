#include "NodeSystem/Node.h"

#include <CppSerializer.h>

#include "NodeSystem/NodeManager.h"

std::string TypeEnumToString(Type type)
{
    switch (type) {
    case Type::Float:
        return "Float";
    case Type::Int:
        return "Int";
    case Type::Bool:
        return "Bool";
    case Type::Vector2:
        return "Vector2";
    case Type::Vector3:
        return "Vector3";
    }
    return "";
}

Input::Input(Type type)
{
    this->index = 0;
    this->type = type;
}

Input::Input(const UUID& _parentUUID, const uint32_t _index, const std::string& _name, const Type _type) : Input(_type)
{
    parentUUID = _parentUUID;
    index = _index;
    name = _name;
    type = _type;
}

#pragma region GetValue
template <typename T>
T Input::GetValue() const
{
}

template <>
int Input::GetValue() const
{
   return static_cast<int>(value[0]); 
}

template <>
bool Input::GetValue() const
{
    return static_cast<bool>(value[0]);
}

template <>
float Input::GetValue() const
{
    return value[0];
}

template <>
Vec2f Input::GetValue() const
{
    return {value.x, value.y};
}

template <>
Vec3f Input::GetValue() const
{
    return value;
}

template <>
Vec4f Input::GetValue() const
{
    return value;
}
#pragma endregion
#pragma region SetValue
template <typename T>
void Input::SetValue(const T& _value)
{
}

template <>
void Input::SetValue(const int& _value)
{
    value[0] = static_cast<float>(_value);
}

template <>
void Input::SetValue(const bool& _value)
{
    value[0] = static_cast<float>(_value);
}

template <>
void Input::SetValue(const float& _value)
{
    value[0] = _value;
}

template <>
void Input::SetValue(const Vec2f& _value)
{
    value.x = _value.x;
    value.y = _value.y;
}

template <>
void Input::SetValue(const Vec3f& _value)
{
    value = _value;
}

template <>
void Input::SetValue(const Vec4f& _value)
{
    value = _value;
}

Input* Input::Clone() const
{
    Input* input = new Input(parentUUID, index, name, type);
    input->SetLinked(isLinked);
    input->value = value;
    return input;
}

uint32_t GetColor(const Type type)
{
    switch (type) {
    case Type::Float:
        return IM_COL32(0, 0, 255, 255);
    case Type::Int:
        return IM_COL32(255, 0, 0, 255);
    case Type::Bool:
        return IM_COL32(0, 255, 0, 255);
    case Type::Vector2:
        return IM_COL32(255, 0, 255, 255);
    case Type::Vector3:
        return IM_COL32(255, 255, 0, 255);
    }
    return 0;
}

Node::Node() : p_nodeManager(nullptr)
{
    
}

Node::Node(std::string _name) : p_nodeManager(nullptr), p_name(std::move(_name))
{
}

void Node::DrawOutputDot(float zoom, const Vec2f& origin, uint32_t i) const
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImFont* font = ImGui::GetIO().Fonts->Fonts[0];
    OutputRef output = p_outputs[i];
    Vec2f position = GetOutputPosition(i, origin, zoom);
    
    uint32_t color = GetColor(output->type);
    drawList->AddCircle(position, streamCircleRadius * zoom, color, 0, 2 * zoom);
    Vec2f textSize = font->CalcTextSizeA(14 * zoom, FLT_MAX, 0.0f, output->name.c_str());
    drawList->AddText(font, 14 * zoom, position + Vec2f(-5, -7.5f) * zoom - Vec2f(textSize.x, 0),
                                  IM_COL32(255, 255, 255, 255), output->name.c_str());
}

void Node::DrawInputDot(float zoom, const Vec2f& origin, uint32_t i) const
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImFont* font = ImGui::GetIO().Fonts->Fonts[0];
    const InputRef input = p_inputs[i];
    Vec2f position = GetInputPosition(i, origin, zoom);
        
    const uint32_t color = GetColor(input->type);
    drawList->AddCircle(position, streamCircleRadius * zoom, color, 0, 2 * zoom);
    drawList->AddText(font, 14 * zoom, position + Vec2f(10, -7.5f) * zoom, IM_COL32(255, 255, 255, 255),
                                  input->name.c_str());
}

void Node::Draw(float zoom, const Vec2f& origin) const
{    
    const auto drawList = ImGui::GetWindowDrawList();
    //Background rect
    Vec2f pMin = GetMin(zoom, origin);
    Vec2f pMax = GetMax(pMin, zoom);
    drawList->AddRectFilled(pMin, pMax, IM_COL32(26, 28, 26, 200), 8.000000, 240);

    // Draw Top
    drawList->AddRectFilled(pMin, pMin + Vec2f(p_size.x, c_topSize) * zoom, p_topColor, 8.000000, 48);
    ImFont* font = ImGui::GetFont();
    drawList->AddText(font, 14 * zoom, pMin + Vec2f(5, 5) * zoom, IM_COL32(255, 255, 255, 255), p_name.c_str());

    if (p_selected)
    {
        drawList->AddRect(pMin, pMax, IM_COL32(255, 255, 0, 255), 8.000000, 240, 2);

        ImGui::SetTooltip(("UUID: " + std::to_string(p_uuid)).c_str());
    }

    for (uint32_t i = 0; i < p_inputs.size(); i++)
    {
        DrawInputDot(zoom, origin, i);
    }

    for (uint32_t i = 0; i < p_outputs.size(); i++)
    {
        DrawOutputDot(zoom, origin, i);
    }
}

bool Node::IsPointHoverCircle(const Vec2f& point, const Vec2f& circlePos, const Vec2f& origin, float zoom, uint32_t index)
{
    Vec2f position = circlePos;
    float radius = (streamCircleRadius + 1.f) * zoom;
    if (point.x > position.x - radius && point.x < position.x + radius && point.y > position.y - radius && point.y <
        position.y + radius)
    {
        return true;
    }
    return false;
}

bool Node::IsSelected(const Vec2f& rectMin, const Vec2f& rectMax, const Vec2f& origin, float zoom) const
{
    Vec2f pMin = GetMin(zoom, origin);
    Vec2f pMax = GetMax(pMin, zoom);
    if (std::min(pMin.x, pMax.x) <= std::max(rectMin.x, rectMax.x)
        && std::max(pMin.x, pMax.x) >= std::min(rectMin.x, rectMax.x)
        && std::min(pMin.y, pMax.y) <= std::max(rectMin.y, rectMax.y)
        && std::max(pMin.y, pMax.y) >= std::min(rectMin.y, rectMax.y))
    {
        return true;
    }
    return false;
}

bool Node::IsNodeVisible(const Vec2f& origin, float zoom) const
{
    Vec2f pMin = GetMin(zoom, origin);
    Vec2f pMax = GetMax(pMin, zoom);
    auto size = ImGui::GetWindowSize();
    auto pos = ImGui::GetWindowPos();
    if (pMax.x > pos.x && pMin.x < pos.x + size.x && pMax.y > pos.y && pMin.y < pos.y + size.y)
    {
        return true;
    }
    return false;
}

bool Node::IsInputClicked(const Vec2f& point, const Vec2f& origin, const float zoom, uint32_t& index) const
{
    Vec2f pMin = GetMin(zoom, origin);
    for (uint32_t i = 0; i < p_inputs.size(); i++)
    {
        if (IsPointHoverCircle(point, GetInputPosition(i, origin, zoom), origin, zoom, i))
        {
            index = i;
            return true;
        }
    }
    return false;
}

bool Node::IsOutputClicked(const Vec2f& point, const Vec2f& origin, float zoom, uint32_t& index) const
{
    Vec2f pMin = GetMin(zoom, origin);
    for (uint32_t i = 0; i < p_outputs.size(); i++)
    {
        Vec2f position = pMin + Vec2f(p_size.x - 10, c_topSize + 10 + c_pointSize * i) * zoom;
        float radius = 5 * zoom;
        if (point.x > position.x - radius && point.x < position.x + radius && point.y > position.y - radius && point.y <
            position.y + radius)
        {
            index = i;
            return true;
        }
    }
    return false;
}

bool Node::DoesOutputHaveLink(const uint32_t index) const
{
    return p_nodeManager->GetLinkManager()->HasLink(p_outputs[index]);
}

bool Node::DoesInputHaveLink(uint32_t index) const
{
    return p_nodeManager->GetLinkManager()->HasLink(p_inputs[index]);
}

void Node::AddInput(const std::string& name, Type type)
{
    p_inputs.push_back(std::make_shared<Input>(p_uuid, static_cast<uint32_t>(p_inputs.size()), name, type));
    int size = 10 + p_inputs.size() * (10 + c_pointSize) + 10;

    if (size > p_size.y)
        p_size.y = static_cast<float>(size);
}

auto Node::AddOutput(const std::string& name, Type type) -> void
{
    p_outputs.push_back(std::make_shared<Output>(p_uuid, static_cast<uint32_t>(p_outputs.size()), name, type));
    int size = 10 + p_outputs.size() * (10 + c_pointSize) + 10;

    if (size > p_size.y)
        p_size.y = static_cast<float>(size);
}

Vec2f Node::GetInputPosition(const uint32_t index, const Vec2f& origin, float zoom) const
{
    return GetMin(zoom, origin) + Vec2f(10, c_topSize + 15 + c_pointSize * index) * zoom;
}

Vec2f Node::GetOutputPosition(const uint32_t index, const Vec2f& origin, float zoom) const
{
    return GetMin(zoom, origin) + Vec2f(p_size.x - 10, c_topSize + 15 + c_pointSize * index) * zoom;
}

std::vector<LinkWeakRef> Node::GetLinks() const
{
    for (auto& link : p_nodeManager->GetLinkManager()->GetLinks())
    {
        if (link->toNodeIndex == p_uuid || link->fromNodeIndex == p_uuid)
        {
            return { link };
        }
    }
    return {};
}

void Node::SetPosition(const Vec2f& position)
{
    p_position = position;
}

void Node::ResetUUID()
{
    SetUUID(UUID());
}

void Node::Serialize(CppSer::Serializer& serializer) const
{
    serializer << CppSer::Pair::BeginMap << "Node";
    serializer << CppSer::Pair::Key << "TemplateID" << CppSer::Pair::Value << p_templateID;
    serializer << CppSer::Pair::Key << "UUID" << CppSer::Pair::Value << p_uuid;
    serializer << CppSer::Pair::Key << "Position" << CppSer::Pair::Value << p_position;
    
    for (uint32_t i = 0; i < p_inputs.size(); i++)
    {
        auto& input = p_inputs[i];
        if (input->isLinked)
            continue;

        serializer << CppSer::Pair::Key << "Value " + std::to_string(i) << CppSer::Pair::Value << input->GetValue();
    }
    
    serializer << CppSer::Pair::EndMap << "Node";
}

void Node::Deserialize(CppSer::Parser& parser)
{
    p_uuid = parser["UUID"].As<uint64_t>();
    SetUUID(p_uuid);
    p_position = parser["Position"].As<Vec2f>();

    for (uint32_t i = 0; i < p_inputs.size(); i++)
    {
        auto& input = p_inputs[i];
        if (input->isLinked)
            continue;

        std::string key = "Value " + std::to_string(i);
        input->SetValue(parser[key].As<Vec4f>());
    }
}

Node* Node::Clone()
{
    auto node = new Node(p_name);
    node->p_inputs = p_inputs;
    node->p_templateID = p_templateID;
    for (size_t i = 0; i < node->p_inputs.size(); i++)
    {
        auto& input = node->p_inputs[i];
        Input* out = nullptr;
        out = input->Clone();
        input = std::shared_ptr<Input>(out);
        input->parentUUID = node->p_uuid;
    }
    node->p_outputs = p_outputs;
    for (auto& output : node->p_outputs)
    {
        output->parentUUID = node->p_uuid;
    }
    node->p_size = p_size;
    node->p_position = p_position;
    node->p_topColor = p_topColor;
    node->p_allowInteraction = p_allowInteraction;
    return node;
}

void Node::SetUUID(const UUID& uuid)
{
    p_uuid = uuid;
    for (auto& input : p_inputs)
    {
        input->parentUUID = p_uuid;
    }
    for (auto& output : p_outputs)
    {
        output->parentUUID = p_uuid;
    }
}
