#include "NodeSystem/NodeTemplateHandler.h"
#include "NodeSystem/Node.h"

#include <ranges>

#include "NodeSystem/CustomNode.h"
#include "NodeSystem/ParamNode.h"

std::unique_ptr<NodeTemplateHandler> NodeTemplateHandler::s_instance;

void NodeTemplateHandler::Initialize()
{
    constexpr ImU32 endColor = IM_COL32(170, 120, 70, 255);         // Muted tan
    constexpr ImU32 functionColor = IM_COL32(90, 130, 180, 255);    // Muted blue
    constexpr ImU32 makeColor = IM_COL32(130, 160, 90, 255);        // Muted olive green
    constexpr ImU32 breakColor = IM_COL32(180, 90, 120, 255);       // Muted pink
    constexpr ImU32 customNodeColor = IM_COL32(110, 150, 170, 255); // Soft teal
    constexpr ImU32 paramNodeColor = IM_COL32(160, 120, 180, 255);  // Muted lavender
    
    {
        NodeRef node = std::make_shared<Node>("Material");
        node->SetTopColor(endColor);
        node->p_allowInteraction = false;
            
        node->AddInput("Base Color", Type::Vector3);
        node->AddInput("Metallic", Type::Float);
        node->AddInput("Specular", Type::Float);
        node->AddInput("Roughness", Type::Float);
        NodeMethodInfo info = node;
        AddTemplateNode(info);
    }

    // TODO : Reed the file with all the templates nodes

#pragma region Custom 
    {
        Ref<ParamNode> node = std::make_shared<ParamNode>("Param");
        node->SetTopColor(paramNodeColor);
        node->SetType(Type::Float);
        NodeMethodInfo info{node};
        AddTemplateNode(info);
    }

    {
        Ref<ParamNode> node = std::make_shared<ParamNode>("TexCoords");
        node->SetTopColor(paramNodeColor);
        node->SetType(Type::Vector2);
        node->SetParamName("TexCoords");
        node->SetEditable(false);
        
        NodeMethodInfo info{node};
        AddTemplateNode(info);
    }

    {
        Ref<ParamNode> node = std::make_shared<ParamNode>("Time");
        node->SetTopColor(paramNodeColor);
        node->SetType(Type::Float);
        node->SetParamName("Time");
        node->SetEditable(false);
        
        NodeMethodInfo info{node};
        AddTemplateNode(info);
    }

    {
        CustomNodeRef node = std::make_shared<CustomNode>("Custom Node");
        node->SetTopColor(customNodeColor);

        node->AddInput("In", Type::Vector3);
        node->AddOutput("Out", Type::Vector3);
        
        NodeMethodInfo info{node};
        AddTemplateNode(info);
    }
#pragma endregion

#pragma region Float
    CreateTemplateNode("Make float", makeColor, { {"Value", Type::Float} }, { {"Result", Type::Float} }, "%s");
    CreateTemplateNode("Add", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "%s + %s");
    CreateTemplateNode("Multiply", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "%s * %s");
    CreateTemplateNode("Subtract", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "%s - %s");
    CreateTemplateNode("Divide", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "%s / %s");
    CreateTemplateNode("One Minus", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "1.0 - %s");
    CreateTemplateNode("Abs", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "abs(%s)");
    CreateTemplateNode("Floor", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "floor(%s)");
    CreateTemplateNode("Ceil", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "ceil(%s)");
    CreateTemplateNode("Round", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "round(%s)");
    CreateTemplateNode("Sign", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "sign(%s)");
    CreateTemplateNode("Sqrt", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "sqrt(%s)");
    CreateTemplateNode("Sin", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "sin(%s)");
    CreateTemplateNode("Cos", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "cos(%s)");
    CreateTemplateNode("Tan", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "tan(%s)");
    CreateTemplateNode("ASin", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "asin(%s)");
    CreateTemplateNode("ACos", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "acos(%s)");
    CreateTemplateNode("ATan", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "atan(%s)");
    CreateTemplateNode("ATan2", functionColor, { {"Y", Type::Float}, {"X", Type::Float} }, { {"Result", Type::Float} }, "atan2(%s, %s)");
    CreateTemplateNode("Mod", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "fmod(%s, %s)");
    CreateTemplateNode("Max", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "max(%s, %s)");
    CreateTemplateNode("Min", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "min(%s, %s)");
    CreateTemplateNode("Clamp", functionColor, { {"Value", Type::Float}, {"Min", Type::Float}, {"Max", Type::Float} }, { {"Result", Type::Float} }, "clamp(%s, %s, %s)");
    CreateTemplateNode("Lerp", functionColor, { {"A", Type::Float}, {"B", Type::Float}, {"T", Type::Float} }, { {"Result", Type::Float} }, "mix(%s, %s, %s)");
    CreateTemplateNode("Smooth Step", functionColor, { {"Edge0", Type::Float}, {"Edge1", Type::Float}, {"Value", Type::Float} }, { {"Result", Type::Float} }, "smoothstep(%s, %s, %s)");
    CreateTemplateNode("Step", functionColor, { {"Edge", Type::Float}, {"Value", Type::Float} }, { {"Result", Type::Float} }, "step(%s, %s)");
    CreateTemplateNode("Fract", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "fract(%s)");
    CreateTemplateNode("Power", functionColor, { {"Base", Type::Float}, {"Exp", Type::Float} }, { {"Result", Type::Float} }, "power(%s)");
    CreateTemplateNode("Log", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "log(%s)");
    CreateTemplateNode("Exp", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "exp(%s)");
    CreateTemplateNode("Log2", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "log2(%s)");
    CreateTemplateNode("Saturate", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "clamp(%s, 0.0, 1.0)");
#pragma endregion

#pragma region Int
    CreateTemplateNode("Make int", makeColor, { {"Value", Type::Int} }, { {"Result", Type::Int} }, "%s");
#pragma endregion

#pragma region Vector2
    CreateTemplateNode("Make Vector2", makeColor, { {"X", Type::Float}, {"Y", Type::Float} }, { {"Result", Type::Vector2} }, "vec2(%s, %s)");
    CreateTemplateNode("Break Vector2", breakColor, { {"Value", Type::Vector2} }, { {"X", Type::Float}, {"Y", Type::Float} }, "%s.x, %s.y");

    {
        NodeRef node = std::make_shared<Node>("Multiply");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector2);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Vector2);
        NodeMethodInfo info = {node, "%s * %s"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Divide");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector2);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Vector2);
        NodeMethodInfo info = {node, "%s / %s"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Frac");
        node->SetTopColor(functionColor);

        node->AddInput("A", Type::Vector2);
        node->AddOutput("Result", Type::Vector2);
        NodeMethodInfo info = {node, "fract(%s)"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Subtract");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector2);
        node->AddInput("B", Type::Vector2);
        node->AddOutput("Result", Type::Vector2);
        NodeMethodInfo info = {node, "%s - %s"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Add");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector2);
        node->AddInput("B", Type::Vector2);
        node->AddOutput("Result", Type::Vector2);
        NodeMethodInfo info = {node, "%s + %s"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Add");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector2);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Vector2);
        NodeMethodInfo info = {node, "%s + %s"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Normalize");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector2);
        node->AddOutput("Result", Type::Vector2);
        NodeMethodInfo info = {node, "normalize(%s)"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Length");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector2);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "length(%s)"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Dot");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector2);
        node->AddInput("B", Type::Vector2);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "dot(%s, %s)"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Cross");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector2);
        node->AddInput("B", Type::Vector2);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "cross(%s, %s)"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("To Vector3");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector2);
        node->AddOutput("Result", Type::Vector3);
        NodeMethodInfo info = {node, "vec3(%s, 0)"};
        AddTemplateNode(info);
    }
#pragma endregion

#pragma region Vector3
    {
        NodeRef node = std::make_shared<Node>("Make Vector3");
        node->SetTopColor(makeColor);
        node->AddInput("X", Type::Float);
        node->AddInput("Y", Type::Float);
        node->AddInput("Z", Type::Float);
        node->AddOutput("Result", Type::Vector3);
        NodeMethodInfo info = {node, "vec3(%s, %s, %s)"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Break Vector3");
        node->SetTopColor(breakColor);
        node->AddInput("Value", Type::Vector3);
        node->AddOutput("X", Type::Float);
        node->AddOutput("Y", Type::Float);
        node->AddOutput("Z", Type::Float);
        NodeMethodInfo info = {node, "%s.x", "%s.y", "%s.z"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Add");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddInput("B", Type::Vector3);
        node->AddOutput("Result", Type::Vector3);
        NodeMethodInfo info = {node, "%s + %s"};
        AddTemplateNode(info);
    }
    {
        NodeRef node = std::make_shared<Node>("Add");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Vector3);
        NodeMethodInfo info = {node, "%s + %s"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Subtract");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddInput("B", Type::Vector3);
        node->AddOutput("Result", Type::Vector3);
        NodeMethodInfo info = {node, "%s - %s"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Subtract");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Vector3);
        NodeMethodInfo info = {node, "%s - %s"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Multiply");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddInput("B", Type::Vector3);
        node->AddOutput("Result", Type::Vector3);
        NodeMethodInfo info = {node, "%s * %s"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Multiply");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Vector3);
        NodeMethodInfo info = {node, "%s * %s"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Saturate");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddOutput("Result", Type::Vector3);
        NodeMethodInfo info = {node, "clamp(%s, 0.0, 1.0)"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("To Vector2");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddOutput("Result", Type::Vector2);
        NodeMethodInfo info = {node, "vec2(%s)"};
        AddTemplateNode(info);
    }
#pragma endregion
    
    // Check if there is duplicate name
    for (uint32_t i = 0; i < m_templateNodes.size(); i++)
    {
        for (uint32_t j = i + 1; j < m_templateNodes.size(); j++)
        {
            if (m_templateNodes[i].node->GetName() == m_templateNodes[j].node->GetName())
            {
                std::string type1 = TypeEnumToString(m_templateNodes[i].node->GetOutput(0)->type);
                std::string type2 = TypeEnumToString(m_templateNodes[j].node->GetOutput(0)->type);
                m_templateNodes[i].node->SetName(m_templateNodes[i].node->GetName() + " (" + type1 + ")");
                m_templateNodes[i].node->p_templateID = TemplateIDFromString(m_templateNodes[i].node->GetName());
                m_templateNodes[j].node->SetName(m_templateNodes[j].node->GetName() + " (" + type2 + ")");
                m_templateNodes[j].node->p_templateID = TemplateIDFromString(m_templateNodes[j].node->GetName());
            }
        }
    }

    // Sort the list by name
    // std::ranges::sort(m_templateNodes, [](const NodeMethodInfo& a, const NodeMethodInfo& b) { return a.node->GetName() < b.node->GetName(); });
}

std::vector<std::string> NodeTemplateHandler::GetTemplateFormatStrings(TemplateID templateID)
{
    for (auto& m_templateNode : s_instance->m_templateNodes)
    {
        if (m_templateNode.node->p_templateID == templateID)
        {
            return m_templateNode.outputFormatStrings;
        }
    }
    return {};
}

void NodeTemplateHandler::AddTemplateNode(const NodeMethodInfo& info)
{
    std::string name = info.node->GetName();
    for (size_t i = 0; i < info.node->p_inputs.size(); i++)
    {
        name += "_" + info.node->p_inputs[i]->name + "_" + TypeEnumToString(info.node->p_inputs[i]->type);
    }
    for (size_t i = 0; i < info.node->p_outputs.size(); i++)
    {
        name += "_" + info.node->p_outputs[i]->name + "_" + TypeEnumToString(info.node->p_outputs[i]->type);
    }
    info.node->p_templateID = TemplateIDFromString(name); 
    m_templateNodes.push_back(info);
}

TemplateID NodeTemplateHandler::TemplateIDFromString(const std::string& name)
{
    return std::hash<std::string>{}(name);
}

std::shared_ptr<Node> NodeTemplateHandler::CreateFromTemplate(size_t templateID)
{
    for (size_t i = 0; i < s_instance->m_templateNodes.size(); i++)
    {
        auto hash = s_instance->m_templateNodes[i].node->GetTemplateID();
        if (hash == templateID)
        {
            return std::shared_ptr<Node>(s_instance->m_templateNodes[i].node->Clone());
        }   
    }
    return nullptr;
}

std::shared_ptr<Node> NodeTemplateHandler::CreateFromTemplateName(const std::string& name)
{
    for (size_t i = 0; i < s_instance->m_templateNodes.size(); i++)
    {
        std::string nodeName = s_instance->m_templateNodes[i].node->GetName();
        if (nodeName == name)
        {
            return std::shared_ptr<Node>(s_instance->m_templateNodes[i].node->Clone());
        }
    }
    return nullptr;
}

void NodeTemplateHandler::CreateTemplateNode(const std::string& name, const uint32_t& color,
    const std::vector<std::tuple<std::string, Type>>& inputs,
    const std::vector<std::tuple<std::string, Type>>& outputs, const std::vector<std::string>& formats)
{
    NodeRef node = std::make_shared<Node>(name);
    node->SetTopColor(color);

    for (auto& input : inputs)
    {
        node->AddInput(std::get<0>(input), std::get<1>(input));
    }
    for (auto& output : outputs)
    {
        node->AddOutput(std::get<0>(output), std::get<1>(output));
    }
    NodeMethodInfo info = {node, formats};
    AddTemplateNode(info);
}

void NodeTemplateHandler::CreateTemplateNode(const std::string& name, const uint32_t& color,
    const std::vector<std::tuple<std::string, Type>>& inputs, const std::vector<std::tuple<std::string, Type>>& outputs,
    const std::string& format)
{
    CreateTemplateNode(name, color, inputs, outputs, std::vector<std::string>{format});
}

void NodeTemplateHandler::CreateTemplateNode(const std::string& name, const uint32_t& color,
    const std::vector<std::string>& inputs, const std::vector<Type>& inputTypes,
    const std::vector<std::string>& outputs, const std::vector<Type>& outputTypes, const std::string& format)
{
    // Iterate over each input type
    for (const auto& inputType : inputTypes)
    {
        // Create a modified inputs list with the corresponding input names and this input type
        std::vector<std::tuple<std::string, Type>> modifiedInputs;
        for (const auto& input : inputs)
        {
            modifiedInputs.emplace_back(input, inputType); // Assign the current input type to all inputs
        }

        // Iterate over each output type
        for (const auto& outputType : outputTypes)
        {
            // Create a modified outputs list with the corresponding output names and this output type
            std::vector<std::tuple<std::string, Type>> modifiedOutputs;
            for (const auto& output : outputs)
            {
                modifiedOutputs.emplace_back(output, outputType); // Assign the current output type to all outputs
            }

            // Use the existing CreateTemplateNode method with modified inputs/outputs
            CreateTemplateNode(name, color, modifiedInputs, modifiedOutputs, format);
        }
    }
}
