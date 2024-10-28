#include "Node.h"

Node::Node(std::string _name) : p_name(_name)
{
}

Node::~Node()
{
}

//TODO implement Node
void Node::Draw(float zoom, const Vec2f& origin)
{
    auto drawList = ImGui::GetWindowDrawList();
    //Background rect
    Vec2f pMin = GetMin(zoom, origin);
    Vec2f pMax = GetMax(pMin, zoom);
    drawList->AddRectFilled(pMin, pMax, IM_COL32(100, 100, 100, 255), 8.000000, 240);
    
    // Draw Top
    drawList->AddRectFilled(pMin, pMin + Vec2f(p_size.x, c_topSize) * zoom, p_topColor, 8.000000, 48);
    ImFont* font = ImGui::GetFont();
    drawList->AddText(font, 14 * zoom, pMin + Vec2f(5, 5) * zoom, IM_COL32(255, 255, 255, 255), p_name.c_str());

    if (p_selected)
    {
        drawList->AddRect(pMin, pMax, IM_COL32(255, 255, 0, 255), 8.000000, 240, 2);
    }

    for (int i = 0; i < p_inputs.size(); i++)
    {
        InputRef input = p_inputs[i];
        Vec2f position = pMin + Vec2f(10, c_topSize + 10 + c_pointSize * i) * zoom;
        drawList->AddCircleFilled(position, 5 * zoom, IM_COL32(200, 200, 200, 255));
        drawList->AddCircle(position, 5 * zoom, IM_COL32(0, 0, 0, 255), 0, 2 * zoom);
        drawList->AddText(font, 14 * zoom, position + Vec2f(5, -7.5f) * zoom, IM_COL32(255, 255, 255, 255), input->name.c_str());
    }

    for (int i = 0; i < p_outputs.size(); i++)
    {
        OutputRef output = p_outputs[i];
        Vec2f position = pMin + Vec2f(p_size.x - 10, c_topSize + 10 + c_pointSize * i) * zoom;
        drawList->AddCircleFilled(position, 5 * zoom, IM_COL32(200, 200, 200, 255));
        drawList->AddCircle(position, 5 * zoom, IM_COL32(0, 0, 0, 255), 0, 2 * zoom);
        Vec2f textSize = font->CalcTextSizeA(14 * zoom, FLT_MAX, 0.0f, output->name.c_str());
        drawList->AddText(font, 14 * zoom, position + Vec2f(-5, -7.5f) * zoom - Vec2f(textSize.x, 0), IM_COL32(255, 255, 255, 255), output->name.c_str());
    }
}

bool Node::IsPointInsideCircle(const Vec2f& point, const Vec2f& origin, float zoom, int index) const
{
    Vec2f pMin = GetMin(zoom, origin);
    Vec2f position = pMin + Vec2f(10, c_topSize + 10 + c_pointSize * index) * zoom;
    float radius = 5 * zoom;
    if (point.x > position.x - radius && point.x < position.x + radius && point.y > position.y - radius && point.y < position.y + radius)
    {
        return true;
    }
    return false;
}

bool Node::IsInputClicked(const Vec2f& point, const Vec2f& origin, float zoom, int& index) const
{
    Vec2f pMin = GetMin(zoom, origin);
    for (int i = 0; i < p_inputs.size(); i++)
    {
        if (IsPointInsideCircle(point, origin, zoom, i))
        {
            index = i;
            return true;
        }
    }
    return false;
}

bool Node::IsOutputClicked(const Vec2f& point, const Vec2f& origin, float zoom, int& index) const
{
    Vec2f pMin = GetMin(zoom, origin);
    for (int i = 0; i < p_outputs.size(); i++)
    {
        Vec2f position = pMin + Vec2f(p_size.x - 10, c_topSize + 10 + c_pointSize * i) * zoom;
        float radius = 5 * zoom;
        if (point.x > position.x - radius && point.x < position.x + radius && point.y > position.y - radius && point.y < position.y + radius)
        {
            index = i;
            return true;
        }
    }
    return false;
}

void Node::AddInput(std::string name, Type type)
{
    p_inputs.push_back(std::make_shared<Input>(name, type));

    uint32_t size = p_inputs.size() * (25 + c_pointSize);

    if (p_outputs.size() * c_pointSize > p_size.y)
        p_size.y = size;
}

void Node::AddOutput(std::string name, Type type)
{
    p_outputs.push_back(std::make_shared<Output>(name, type));
    uint32_t size = p_outputs.size() * (25 + c_pointSize);

    if (p_outputs.size() * c_pointSize > p_size.y)
        p_size.y = size;
}

void Node::SetPosition(Vec2f position)
{
    p_position = position;
}
