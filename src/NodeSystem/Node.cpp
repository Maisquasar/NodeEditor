#include "NodeSystem/Node.h"

#include <CppSerializer.h>
#include <imgui_internal.h>

#include "NodeWindow.h"
#include "Actions/Action.h"
#include "Actions/ActionChangeValue.h"
#include "NodeSystem/NodeManager.h"
#include "NodeSystem/NodeTemplateHandler.h"
#include "NodeSystem/ShaderMaker.h"
#include "Render/Font.h"
#include "Render/Framebuffer.h"

const char* SerializeTypeEnum()
{
    return "Float\0Int\0Bool\0Vector2\0Vector3\0Vector4";
}

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
    case Type::Vector4:
        return "Vector4";
    }
    return "";
}

std::unordered_map<Type, uint32_t> colorForType
{
        { Type::Float, IM_COL32(90, 120, 255, 255) },    // Soft blue
        { Type::Int, IM_COL32(255, 100, 100, 255) },     // Soft red
        { Type::Bool, IM_COL32(100, 255, 100, 255) },    // Soft green
        { Type::Vector2, IM_COL32(180, 90, 255, 255) },  // Soft purple
        { Type::Vector3, IM_COL32(255, 255, 90, 255) },  // Soft yellow
        { Type::Vector4, IM_COL32(90, 255, 255, 255) }   // Soft cyan
};

uint32_t GetColorFromType(Type type)
{
    if (colorForType.find(type) != colorForType.end())
        return colorForType[type];
    return IM_COL32(255, 255, 255, 255);
}

void AddColor(uint32_t& color, int value)
{
    // Extract individual color channels using bitwise operations
    ImU32 r = (color >> IM_COL32_R_SHIFT) & 0xFF;
    ImU32 g = (color >> IM_COL32_G_SHIFT) & 0xFF;
    ImU32 b = (color >> IM_COL32_B_SHIFT) & 0xFF;
    ImU32 a = (color >> IM_COL32_A_SHIFT) & 0xFF; // Alpha remains unchanged

    // Add brightness
    r += value;
    g += value;
    b += value;

    // Clamp to 255
    r = std::clamp(r, static_cast<ImU32>(0), static_cast<ImU32>(255));
    g = std::clamp(g, static_cast<ImU32>(0), static_cast<ImU32>(255));
    b = std::clamp(b, static_cast<ImU32>(0), static_cast<ImU32>(255));

    // Reconstruct the color with clamped values
    color = IM_COL32(r, g, b, a);
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

void DrawTriangle(ImDrawList* draw_list, ImGuiDir dir, const Vec2f& position, const Vec2f& size)
{
    Vec2f p1, p2, p3;

    switch (dir)
    {
    case ImGuiDir_Up:
        p1 = position + Vec2f(size.x * 0.5f, 0);
        p2 = position + Vec2f(0, size.y);
        p3 = position + Vec2f(size.x, size.y);
        break;
    case ImGuiDir_Down:
        p1 = position + Vec2f(0, 0);
        p2 = position + Vec2f(size.x, 0);
        p3 = position + Vec2f(size.x * 0.5f, size.y);
        break;
    case ImGuiDir_Left:
        p1 = position + Vec2f(size.x, 0);
        p2 = position + Vec2f(size.x, size.y);
        p3 = position + Vec2f(0, size.y * 0.5f);
        break;
    case ImGuiDir_Right:
        p1 = position + Vec2f(0, 0);
        p2 = position + Vec2f(size.x, size.y * 0.5f);
        p3 = position + Vec2f(0, size.y);
        break;
    default:
        return;
    }

    draw_list->AddTriangleFilled(p1, p2, p3, IM_COL32(255, 255, 255, 255));
}

Node::Node() : p_nodeManager(nullptr)
{
    
}

Node::Node(std::string _name) : p_nodeManager(nullptr), p_name(std::move(_name))
{
}

void Node::DrawDot(float zoom, const Vec2f& origin, uint32_t i, bool isOutput) const
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImFont* font = Font::GetFontScaled();
    ImFont* fontToCalc = Font::GetFont();

    // Get the relevant data
    const auto& ref = isOutput ? static_cast<const StreamRef&>(p_outputs[i]) : static_cast<const StreamRef&>(p_inputs[i]);
    Vec2f position = isOutput ? GetOutputPosition(i, origin, zoom) : GetInputPosition(i, origin, zoom);

    // Get color and size
    uint32_t color = GetColorFromType(ref->type);
    float dotSize = c_streamCircleRadius * zoom;
    if (ref->isHovered && !ref->isLinked)
        dotSize *= c_hoveredCircleRadiusFactor;
    else if (ref->isHovered && ref->isLinked)
        AddColor(color, -50);

    // Draw the dot
    drawList->AddCircle(position, dotSize, color, 0, 2 * zoom);

    // Calculate text position offset
    Vec2f textOffset = isOutput ? Vec2f(-10, -7.5f) : Vec2f(10, -7.5f);
    Vec2f textSize = fontToCalc->CalcTextSizeA(14 * zoom, FLT_MAX, 0.0f, ref->name.c_str());

    Vec2f textOff = {};
    if (isOutput)
        textOff.x = textSize.x;
    // Draw the text
    drawList->AddText(font, 14 * zoom, position + textOffset * zoom - textOff, IM_COL32(255, 255, 255, 255), ref->name.c_str());
}

void Node::DrawOutputDot(float zoom, const Vec2f& origin, uint32_t i) const
{
    DrawDot(zoom, origin, i, true);
}

void Node::DrawInputDot(float zoom, const Vec2f& origin, uint32_t i) const
{
    DrawDot(zoom, origin, i, false);
}

void Node::DrawButtonPreview(float zoom, const Vec2f& origin, Vec2f pMin, Vec2f pMax) const
{
    auto drawList = ImGui::GetWindowDrawList();
    Vec2f triangleSize, trianglePos;
    GetPreviewTriangle(trianglePos, triangleSize, pMin, pMax, zoom);

    if (p_previewHovered)
    {
        Vec2f rectMin, rectMax;
        GetPreviewButtonRect(rectMin, rectMax, pMin, pMax, zoom);
        drawList->AddRect(rectMin, rectMax, IM_COL32(255, 255, 255, 255));
    }
    DrawTriangle(drawList, p_preview ? ImGuiDir_Up : ImGuiDir_Down, trianglePos, triangleSize);
}

void Node::Update()
{
}

void Node::GetPreviewRect(const Vec2f& pMin, float zoom, Vec2f& imageMin, Vec2f& imageMax) const
{
    float gap = 5.0f; // Gap around the image

    float sizeY = ((p_sizeWithPreview.y - p_size.y) - 2 * gap) * zoom;
    float sizeX = sizeY; // Maintain 1:1 aspect ratio

    // Center horizontally, considering zoom and gap
    float centerXOffset = (p_size.x * zoom - sizeX) * 0.5f;

    imageMin = pMin + Vec2f(centerXOffset, p_size.y * zoom + gap * zoom);
    imageMax = imageMin + Vec2f(sizeX, sizeY);
}

void Node::DrawPreview(Vec2f pMin, float zoom) const
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    Vec2f imageMin;
    Vec2f imageMax;
    GetPreviewRect(pMin, zoom, imageMin, imageMax);

    if (m_framebuffer)
    {
        m_framebuffer->SetNewSize(imageMax - imageMin);
    }

    drawList->AddImage(reinterpret_cast<ImTextureID>(m_framebuffer->GetRenderTexture()), imageMin, imageMax, ImVec2(0, 1), ImVec2(1, 0));
}

void Node::Draw(float zoom, const Vec2f& origin) const
{
    const auto drawList = ImGui::GetWindowDrawList();
    //Background rect
    Vec2f pMin = GetMin(zoom, origin);
    Vec2f pMax = GetMax(pMin, zoom);
    drawList->AddRectFilled(pMin, pMax, IM_COL32(26, 28, 26, 200), 8.f * zoom, 240);

    // Draw Top
    drawList->AddRectFilled(pMin, pMin + Vec2f(p_size.x, c_topSize) * zoom, p_topColor, 8.f * zoom, 48);
    ImFont* font = Font::GetFontScaled();
    drawList->AddText(font, 14 * zoom, pMin + Vec2f(5, 5) * zoom, IM_COL32(255, 255, 255, 255), p_name.c_str());

    DrawButtonPreview(zoom, origin, pMin, pMax);

#ifdef _DEBUG
    bool hover = false;
    for (uint32_t i = 0; i < p_inputs.size() + p_outputs.size(); i++)
    {
        StreamRef ref;
        if (i < p_inputs.size())
        {
            ref = p_inputs[i];
        }
        else
        {
            ref = p_outputs[i - p_inputs.size()];
        }
        if (ref->isHovered)
        {
            hover = true;
            ImGui::SetTooltip(("UUID: " + std::to_string(ref->parentUUID)).c_str());
            break;
        }
    }
#endif
    if (p_selected)
    {
        drawList->AddRect(pMin, pMax, IM_COL32(255, 255, 0, 255), 8.f * zoom, 240, 2);

#ifdef _DEBUG
        if (!hover)
        {
            ImGui::SetTooltip(("UUID: " + std::to_string(p_uuid)).c_str());
        }
#endif
    }

    for (uint32_t i = 0; i < p_inputs.size(); i++)
    {
        DrawInputDot(zoom, origin, i);
    }

    for (uint32_t i = 0; i < p_outputs.size(); i++)
    {
        DrawOutputDot(zoom, origin, i);
    }

    if (p_preview)
    {
        DrawPreview(pMin, zoom);
    }
}

bool Node::IsPointHoverCircle(const Vec2f& point, const Vec2f& circlePos, const Vec2f& origin, float zoom, uint32_t index)
{
    Vec2f position = circlePos;
    float radius = (c_streamCircleRadius * c_hoveredCircleRadiusFactor) * zoom;
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

bool Node::IsPreviewHovered(const Vec2f& point, const Vec2f& origin, float zoom) const
{
    Vec2f pMin = GetMin(zoom, origin);
    Vec2f pMax = GetMax(pMin, zoom);
    Vec2f rectMin, rectMax;
    GetPreviewButtonRect(rectMin, rectMax, pMin, pMax, zoom);

    if (point.x > rectMin.x && point.x < rectMax.x && point.y > rectMin.y && point.y < rectMax.y)
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

int Node::CalculateSize(int size)
{
    return 15 + size * (0 + c_pointSize) + 15;
}

void Node::AddInput(const std::string& name, Type type)
{
    p_inputs.push_back(std::make_shared<Input>(p_uuid, static_cast<uint32_t>(p_inputs.size()), name, type));
    int size = CalculateSize(p_inputs.size());

    p_size.y = std::max(p_size.y, static_cast<float>(size));
}

auto Node::AddOutput(const std::string& name, Type type) -> void
{
    p_outputs.push_back(std::make_shared<Output>(p_uuid, static_cast<uint32_t>(p_outputs.size()), name, type));
    int size = CalculateSize(p_outputs.size());

    p_size.y = std::max(p_size.y, static_cast<float>(size));
}

void Node::RemoveInput(uint32_t index)
{
    p_inputs.erase(p_inputs.begin() + index);
    int size = CalculateSize(p_inputs.size());

    int size2 = CalculateSize(p_outputs.size());

    auto linkManager = p_nodeManager->GetLinkManager();
    for (auto& link : linkManager->GetLinks())
    {
        if (link->toNodeIndex == p_uuid && link->toInputIndex == index)
        {
            linkManager->RemoveLink(link->toInputIndex, false);
            break;
        }
    }
    
    p_size.y = static_cast<float>(std::max(size, size2));
}

void Node::RemoveOutput(uint32_t index)
{
    if (!p_nodeManager) // Case when Using with a templateNode
        return;
    int size = CalculateSize(p_inputs.size());

    int size2 = CalculateSize(p_outputs.size());
    
    auto linkManager = p_nodeManager->GetLinkManager();
    linkManager->RemoveLinks(p_outputs[index]);

    p_size.y = static_cast<float>(std::max(size, size2));
    p_outputs.erase(p_outputs.begin() + index);
}

Vec2f Node::GetInputPosition(const uint32_t index, const Vec2f& origin, float zoom) const
{
    return GetMin(zoom, origin) + Vec2f(10, c_topSize + 15 + c_pointSize * index) * zoom;
}

Vec2f Node::GetInputPosition(uint32_t index) const
{
    return p_position + Vec2f(10, c_topSize + 15 + c_pointSize * index);
}

Vec2f Node::GetOutputPosition(const uint32_t index, const Vec2f& origin, float zoom) const
{
    return GetMin(zoom, origin) + Vec2f(p_size.x - 10, c_topSize + 15 + c_pointSize * index) * zoom;
}

Vec2f Node::GetOutputPosition(uint32_t index) const
{
    return p_position + Vec2f(p_size.x - 10, c_topSize + 15 + c_pointSize * index);
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

void Node::ComputeNodeSize()
{
    float textSizeX = ImGui::CalcTextSize(p_name.c_str()).x + 20.f;
    p_size.x = std::max(textSizeX, p_size.x);
}

void Node::GetPreviewTriangle(Vec2f& trianglePos, Vec2f& triangleSize, const Vec2f& nodeMin, const Vec2f& nodeMax, float zoom)
{
    triangleSize = Vec2f(10) * zoom;
    trianglePos = Vec2f(nodeMax.x, nodeMin.y) - Vec2f(triangleSize.x * 2.f, -(c_topSize * zoom - triangleSize.y) * 0.5f);
}

void Node::GetPreviewButtonRect(Vec2f& outMin, Vec2f& outMax, const Vec2f& nodeMin, const Vec2f& nodeMax, float zoom)
{
    Vec2f triangleSize;
    Vec2f trianglePos;
    
    GetPreviewTriangle(trianglePos, triangleSize, nodeMin, nodeMax, zoom);

    const float gap = 5.0f;
    Vec2f rectPos = trianglePos - Vec2f(gap) * zoom;
    Vec2f rectSize = triangleSize + Vec2f(gap * 2) * zoom;

    outMin = rectPos;
    outMax = rectPos + rectSize;
}

void Node::ShowInInspector()
{
    ImGui::PushID(GetUUID());
    ImGui::Text("Inputs:");
    for (uint32_t i = 0; i < p_inputs.size(); ++i)
    {
            ImGui::PushID(i);
        InputRef input = GetInput(i);
        if (input->isLinked)
            continue;
        ImGui::Text("%s: ", input->name.c_str());
        ImGui::SameLine();
                
        Vec4f prevValue = input->GetValue<Vec4f>();
        switch (input->type)
        {
        case Type::Float:
            {
                float value = input->GetValue<float>();
                if (ImGui::DragFloat("##float", &value, 0.1f, FLT_MIN, FLT_MAX, "%.2f"))
                {
                    input->SetValue<float>(value);
                    auto action = std::make_shared<ActionChangeValue>(prevValue, input->GetValue<Vec4f>(), &input->value);
                    ActionManager::AddAction(action);
                }
                break;
            }
        case Type::Int:
            {
                int value = input->GetValue<int>();
                if (ImGui::InputInt("##int", &value))
                {
                    input->SetValue<int>(value);
                    auto action = std::make_shared<ActionChangeValue>(prevValue, input->GetValue<Vec4f>(), &input->value);
                    ActionManager::AddAction(action);
                }
                break;
            }
        case Type::Bool:
            {
                bool value = input->GetValue<bool>();
                if (ImGui::Checkbox("##bool", &value))
                {
                    input->SetValue<bool>(value);
                    auto action = std::make_shared<ActionChangeValue>(prevValue, input->GetValue<Vec4f>(), &input->value);
                    ActionManager::AddAction(action);
                }
                break;
            }
        case Type::Vector2:
            {
                Vec2f value = input->GetValue<Vec2f>();
                if (ImGui::InputFloat2("##vec2", &value[0]))
                {
                    input->SetValue<Vec2f>(value);
                    auto action = std::make_shared<ActionChangeValue>(prevValue, input->GetValue<Vec4f>(), &input->value);
                    ActionManager::AddAction(action);
                }
                break;
            }
        case Type::Vector3:
            {
                Vec3f value = input->GetValue<Vec3f>();
                if (ImGui::InputFloat3("##vec3", &value.x))
                {
                    input->SetValue<Vec3f>(value);
                    auto action = std::make_shared<ActionChangeValue>(prevValue, input->GetValue<Vec4f>(), &input->value);
                    ActionManager::AddAction(action);
                }
                input->SetValue<Vec3f>(value);
                break;
            }
        case Type::Vector4:
            {
                Vec4f value = input->GetValue<Vec4f>();
                if (ImGui::InputFloat4("##vec4", &value.x))
                {
                    input->SetValue<Vec4f>(value);
                    auto action = std::make_shared<ActionChangeValue>(prevValue, input->GetValue<Vec4f>(), &input->value);
                    ActionManager::AddAction(action);
                }
                break;
            }
        default:
            ImGui::Text("Unknown type");
            break;
        }
        ImGui::PopID();
    }
    ImGui::PopID();
}

std::vector<std::string> Node::GetFormatStrings() const
{
    return NodeTemplateHandler::GetTemplateFormatStrings(p_templateID);
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

    InternalSerialize(serializer);
    
    serializer << CppSer::Pair::EndMap << "Node";
}

void Node::InternalSerialize(CppSer::Serializer& serializer) const
{
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

    InternalDeserialize(parser);
}

void Node::InternalDeserialize(CppSer::Parser& parser)
{
}


std::string Node::ToShader(ShaderMaker* shaderMaker, const FuncStruct& funcStruct) const
{
    std::string content;
    auto& variableNames = shaderMaker->m_variablesNames;
    auto& allVariableNames = shaderMaker->m_allVariableNames;
    auto toFormatList = GetFormatStrings();
    for (int k = 0; k < p_outputs.size(); k++)
    {
        if (funcStruct.outputs.size() <= k)
            break;
        std::string variableName = funcStruct.outputs[k];
        std::string glslType = ShaderMaker::TypeToGLSLType(p_outputs[k]->type);
            
        std::string thisContent = glslType + " " + variableName + " = ";
        if (allVariableNames.contains(variableName))
            continue;
        allVariableNames.insert(variableName);
            
        std::string toFormat = toFormatList[k];
        std::string secondHalf = toFormat;
        toFormat.clear();
        for (int j = 0; j < p_inputs.size(); j++)
        {
            InputRef input = p_inputs[j];
            size_t index = secondHalf.find_first_of("%") + 2;
            if (index == std::string::npos)
                break;
            std::string firstHalf = secondHalf.substr(0, index);
            if (index != std::string::npos)
                secondHalf = secondHalf.substr(index);
            if (!input->isLinked)
            {
                variableNames.push_back(ShaderMaker::GetValueAsString(input));
            }
            auto parentVariableName = funcStruct.inputs[j];
            if (parentVariableName.empty())
                parentVariableName = variableNames.back();
            toFormat += FormatString(firstHalf, parentVariableName.c_str());
        }
        thisContent += toFormat + secondHalf + ";\n";

        content += thisContent;

        // std::cout << thisContent << '\n';
    }
    return content;
}

Node* Node::Clone() const
{
    auto node = new Node(p_name);
    Internal_Clone(node);
    return node;
}

void Node::OnChangeUUID(const UUID& prevUUID, const UUID& newUUID)
{
}

void Node::OpenPreview(bool open)
{
    p_preview = open;

    if (p_preview)
    {
        p_nodeManager->GetMainWindow()->AddPreviewNode(p_uuid);
        p_sizeWithPreview = {p_size.x, p_size.y + p_size.x};

        if (!m_shader)
        {
            m_shader = std::make_shared<Shader>();
            m_framebuffer = std::make_shared<Framebuffer>();

            m_shader->LoadDefaultShader();
            m_framebuffer->Initialize();
            p_nodeManager->GetMainWindow()->ShouldUpdateShader();
        }
    }
    else
    {
        p_nodeManager->GetMainWindow()->RemovePreviewNode(p_uuid);
    }
}

void Node::SetUUID(const UUID& uuid)
{
    OnChangeUUID(p_uuid, uuid);
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

void Node::Internal_Clone(Node* node) const
{
    node->p_name = p_name;
    node->p_inputs = p_inputs;
    node->p_templateID = p_templateID;
    node->p_computed = p_computed;
    for (size_t i = 0; i < node->p_inputs.size(); i++)
    {
        auto& input = node->p_inputs[i];
        Input* out = nullptr;
        out = input->Clone();
        input = std::shared_ptr<Input>(out);
        input->parentUUID = node->p_uuid;
    }
    node->p_outputs = p_outputs;
    for (size_t i = 0; i < node->p_outputs.size(); i++)
    {
        auto& output = node->p_outputs[i];
        output = std::make_shared<Output>(*output);
        output->parentUUID = node->p_uuid;
    }
    node->p_size = p_size;
    node->p_position = p_position;
    node->p_topColor = p_topColor;
    node->p_allowInteraction = p_allowInteraction;
}
