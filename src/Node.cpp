#include "Node.h"

#include "NodeManager.h"

uint32_t GetColor(const Type type)
{
    switch (type) {
    case Type::Float:
        return IM_COL32(0, 0, 255, 255);
    case Type::Int:
        return IM_COL32(255, 0, 0, 255);
    case Type::Bool:
        return IM_COL32(0, 255, 0, 255);
    case Type::String:
        return IM_COL32(0, 255, 255, 255);
    case Type::Vector2:
        return IM_COL32(255, 0, 255, 255);
    case Type::Vector3:
        return IM_COL32(255, 255, 0, 255);
    }
    return 0;
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
    
    if (DoesOutputHaveLink(i))
        drawList->AddCircleFilled(position, 5 * zoom, IM_COL32(200, 200, 200, 255));
    
    uint32_t color = GetColor(output->type);
    drawList->AddCircle(position, 5 * zoom, color, 0, 2 * zoom);
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
        
    if (DoesInputHaveLink(i))
        drawList->AddCircleFilled(position, 5 * zoom, IM_COL32(200, 200, 200, 255));
        
    const uint32_t color = GetColor(input->type);
    drawList->AddCircle(position, 5 * zoom, color, 0, 2 * zoom);
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
    float radius = 5 * zoom;
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
    int size = p_inputs.size() * (10 + c_pointSize);

    if (size > p_size.y)
        p_size.y = size;
}

auto Node::AddOutput(const std::string& name, Type type) -> void
{
    p_outputs.push_back(std::make_shared<Output>(p_uuid, static_cast<uint32_t>(p_outputs.size()), name, type));
    int size = p_outputs.size() * (25 + c_pointSize);

    if (p_outputs.size() * c_pointSize > p_size.y)
        p_size.y = static_cast<float>(size);
}

Vec2f Node::GetInputPosition(const uint32_t index, const Vec2f& origin, float zoom) const
{
    return GetMin(zoom, origin) + Vec2f(10, c_topSize + 15 + c_pointSize * index) * zoom;
}

Vec2f Node::GetOutputPosition(const uint32_t index, const Vec2f& origin, float zoom) const
{
    return GetMin(zoom, origin) + Vec2f(p_size.x - 10, c_topSize + 10 + c_pointSize * index) * zoom;
}

LinkWeakRef Node::GetLinkWithOutput(const uint32_t index) const
{
    return p_nodeManager->GetLinkWithOutput(p_uuid, index);
}

std::vector<LinkWeakRef> Node::GetLinksWithInput(uint32_t index) const
{
    return p_nodeManager->GetLinkManager()->GetLinksWithInput(p_uuid, index);
}

void Node::SetPosition(const Vec2f position)
{
    p_position = position;
}
