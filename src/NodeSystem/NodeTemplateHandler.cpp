#include "NodeSystem/NodeTemplateHandler.h"

#include <map>

#include "NodeSystem/Node.h"

#include <ranges>

#include "NodeSystem/CustomNode.h"
#include "NodeSystem/ParamNode.h"
#include "NodeSystem/RerouteNodeNamed.h"

#include "NodeSystem/ShaderMaker.h"
#include "Render/Framebuffer.h"

class Shader;
std::unique_ptr<NodeTemplateHandler> NodeTemplateHandler::s_instance;

NodeMethodInfo::NodeMethodInfo(NodeRef ref, std::vector<std::string> _formatStrings): node(ref)
{
    outputFormatStrings.push_back(_formatStrings);
}

constexpr ImU32 endColor = IM_COL32(170, 120, 70, 255);         // Muted tan
constexpr ImU32 functionColor = IM_COL32(90, 130, 180, 255);    // Muted blue
constexpr ImU32 makeColor = IM_COL32(130, 160, 90, 255);        // Muted olive green
constexpr ImU32 breakColor = IM_COL32(180, 90, 120, 255);       // Muted pink
constexpr ImU32 customNodeColor = IM_COL32(110, 150, 170, 255); // Soft teal
constexpr ImU32 paramNodeColor = IM_COL32(160, 120, 180, 255);  // Muted lavender
constexpr ImU32 otherNodeColor = IM_COL32(180, 90, 120, 255);   // Muted pink

const std::map<NodeColorType, uint32_t> nodeColors = {
    {NodeColorType::EndColor, endColor},
    {NodeColorType::FunctionColor, functionColor},
    {NodeColorType::MakeColor, makeColor},
    {NodeColorType::BreakColor, breakColor},
    {NodeColorType::CustomNodeColor, customNodeColor},
    {NodeColorType::ParamNodeColor, paramNodeColor},
    {NodeColorType::OtherNodeColor, otherNodeColor}
};

uint32_t NodeTemplateHandler::GetNodeColor(const NodeColorType type)
{
    return nodeColors.at(type);
}

void NodeTemplateHandler::RunUnitTests()
{
    for (auto& node : m_templateNodes)
    {
        assert(RunUnitTest(node));
    }
    std::cout << "NodeTemplateHandler::RunUnitTests() passed\n";
}

bool NodeTemplateHandler::RunUnitTest(const NodeMethodInfo& info)
{
    Ref<Node> node = info.node;
    if (!node->p_allowInteraction || typeid(*node) != typeid(Node))
        return true;

    for (int l = 0; l < node->p_possiblityCount; l++)
    {
        node->ConvertStream(l);
        Ref<Shader> shader = std::make_shared<Shader>();
        if (!shader->LoadDefaultVertex())
        {
            std::cout << "Failed to load vertex shader\n";
            return false;
        }
    
        std::string content;
        content += "#version 330 core\nvoid main()\n{\n";
        std::vector<std::string> inputNames(node->p_inputs.size());
        for (int i = 0; i < node->p_inputs.size(); i++)
        {
            Ref<Input> input = node->p_inputs[i];
            auto stringType = ShaderMaker::TypeToGLSLType(input->possibleTypes[l]);
            std::string inputName = input->name + "_" + std::to_string(i);
            content += stringType + " " + inputName + ";\n";
            inputNames[i] = inputName;
        }

        auto formatList = node->GetFormatStrings();
        for (int k = 0; k <  node->p_outputs.size(); k++)
        {
            std::string variableName = node->p_outputs[k]->name + "_" + std::to_string(k);
            std::string glslType = ShaderMaker::TypeToGLSLType(node->p_outputs[k]->possibleTypes[l]);
            
            std::string thisContent = "\t" + glslType + " " + variableName + " = ";
            std::string toFormat = formatList[k];
            std::string secondHalf = toFormat;
            toFormat.clear();
            for (int j = 0; j < node->p_inputs.size(); j++)
            {
                InputRef input = node->p_inputs[j];
                size_t index = secondHalf.find_first_of("%") + 2;
                if (index == std::string::npos)
                    break;
                std::string firstHalf = secondHalf.substr(0, index);
                if (index != std::string::npos)
                    secondHalf = secondHalf.substr(index);
                const auto& parentVariableName =inputNames[j];
                toFormat += FormatString(firstHalf, parentVariableName.c_str());
            }
            thisContent += toFormat + secondHalf + ";\n";

            content += thisContent;
        }
        content += "}\n";
        bool success = shader->SetFragmentShaderContent(content);
        if (!success)
        {
            std::string debugName = node->GetName();
            for (int i = 0; i < node->p_inputs.size(); i++)
            {
                debugName += " " + TypeEnumToString(node->p_inputs[i]->type);
            }
            for (int i = 0; i < node->p_outputs.size(); i++)
            {
                debugName += " " + TypeEnumToString(node->p_outputs[i]->type);
            }
            std::cout << "Failed with node: " << debugName << std::endl;
            return false;
        }
    
        if (!shader->Link())
            return false;
    }
    node->ConvertStream(0);
    return true;
}

void NodeTemplateHandler::Initialize()
{
    
    {
        NodeRef node = std::make_shared<Node>("Material");
        node->SetTopColor(endColor);
        node->p_allowInteraction = false;
            
        node->AddInput("Base Color", Type::Vector3);
        node->AddInput("Metallic", Type::Float);
        node->AddInput("Roughness", Type::Float);
        node->AddInput("Specular", Type::Float);
        NodeMethodInfo info = node;
        info.outputFormatStrings.push_back({"%s"});
        AddTemplateNode(info);
    }

#pragma region Custom 
    {
        Ref<ParamNode> node = std::make_shared<ParamNode>("Param");
        node->SetTopColor(paramNodeColor);
        node->SetType(Type::Float);
        node->p_alwaysVisibleOnContext = true;
        NodeMethodInfo info{node};
        AddTemplateNode(info);
    }

    {
        CustomNodeRef node = std::make_shared<CustomNode>("Custom Node");
        node->SetTopColor(customNodeColor);

        node->AddInput("In", Type::Vector3);
        node->AddOutput("Out", Type::Vector3);
        node->p_alwaysVisibleOnContext = true;
        
        NodeMethodInfo info{node};
        AddTemplateNode(info);
    }

    {
        Ref<RerouteNodeNamed> node = std::make_shared<RerouteNodeNamed>("Reroute Named");
        node->SetTopColor(customNodeColor);
        node->AddInput("", Type::Float);
        node->AddOutput("", Type::Float);
        node->p_alwaysVisibleOnContext = true;
        
        NodeMethodInfo info{node};
        AddTemplateNode(info);
    }
#pragma endregion
    CreateTemplateNode("Add", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "%s + %s", {"+"});
    CreateTemplateNode("Add", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "%s + %s", {"+"});
    CreateTemplateNode("Add", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "%s + %s", {"+"});
    CreateTemplateNode("Add", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "%s + %s", {"+"});
    CreateTemplateNode("Add", functionColor, { {"A", Type::Vector2}, {"B", Type::Float} }, { {"Result", Type::Vector2} }, "%s + %s", {"+"});
    CreateTemplateNode("Add", functionColor, { {"A", Type::Vector3}, {"B", Type::Float} }, { {"Result", Type::Vector3} }, "%s + %s", {"+"});
    CreateTemplateNode("Add", functionColor, { {"A", Type::Vector4}, {"B", Type::Float} }, { {"Result", Type::Vector4} }, "%s + %s", {"+"});
    CreateTemplateNode("Add", functionColor, { {"A", Type::Float}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "%s + %s", {"+"});
    CreateTemplateNode("Add", functionColor, { {"A", Type::Float}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "%s + %s", {"+"});
    CreateTemplateNode("Add", functionColor, { {"A", Type::Float}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "%s + %s", {"+"});

    CreateTemplateNode("Subtract", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "%s - %s", {"-"});
    CreateTemplateNode("Subtract", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "%s - %s", {"-"});
    CreateTemplateNode("Subtract", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "%s - %s", {"-"});
    CreateTemplateNode("Subtract", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "%s - %s", {"-"});
    CreateTemplateNode("Subtract", functionColor, { {"A", Type::Vector2}, {"B", Type::Float} }, { {"Result", Type::Vector2} }, "%s - %s", {"-"});
    CreateTemplateNode("Subtract", functionColor, { {"A", Type::Vector3}, {"B", Type::Float} }, { {"Result", Type::Vector3} }, "%s - %s", {"-"});
    CreateTemplateNode("Subtract", functionColor, { {"A", Type::Vector4}, {"B", Type::Float} }, { {"Result", Type::Vector4} }, "%s - %s", {"-"});
    CreateTemplateNode("Subtract", functionColor, { {"A", Type::Float}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "%s - %s", {"-"});
    CreateTemplateNode("Subtract", functionColor, { {"A", Type::Float}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "%s - %s", {"-"});
    CreateTemplateNode("Subtract", functionColor, { {"A", Type::Float}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "%s - %s", {"-"});

    CreateTemplateNode("One Minus", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "1.0 - %s", {"1 - "});
    CreateTemplateNode("One Minus", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "vec2(1.0) - %s", {"1 - "});
    CreateTemplateNode("One Minus", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "vec3(1.0) - %s", {"1 - "});
    CreateTemplateNode("One Minus", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "vec4(1.0) - %s", {"1 - "});

    CreateTemplateNode("Multiply", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "%s * %s", {"*"});
    CreateTemplateNode("Multiply", functionColor, { {"A", Type::Vector2}, {"B", Type::Float} }, { {"Result", Type::Vector2} }, "%s * %s", {"*"});
    CreateTemplateNode("Multiply", functionColor, { {"A", Type::Vector3}, {"B", Type::Float} }, { {"Result", Type::Vector3} }, "%s * %s", {"*"});
    CreateTemplateNode("Multiply", functionColor, { {"A", Type::Vector4}, {"B", Type::Float} }, { {"Result", Type::Vector4} }, "%s * %s", {"*"});
    CreateTemplateNode("Multiply", functionColor, { {"A", Type::Float}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "%s * %s", {"*"});
    CreateTemplateNode("Multiply", functionColor, { {"A", Type::Float}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "%s * %s", {"*"});
    CreateTemplateNode("Multiply", functionColor, { {"A", Type::Float}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "%s * %s", {"*"});
    CreateTemplateNode("Multiply", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "%s * %s", {"*"});
    CreateTemplateNode("Multiply", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "%s * %s", {"*"});
    CreateTemplateNode("Multiply", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "%s * %s", {"*"});

    CreateTemplateNode("Divide", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "%s / %s", {"/"});
    CreateTemplateNode("Divide", functionColor, { {"A", Type::Vector2}, {"B", Type::Float} }, { {"Result", Type::Vector2} }, "%s / %s", {"/"});
    CreateTemplateNode("Divide", functionColor, { {"A", Type::Vector3}, {"B", Type::Float} }, { {"Result", Type::Vector3} }, "%s / %s", {"/"});
    CreateTemplateNode("Divide", functionColor, { {"A", Type::Vector4}, {"B", Type::Float} }, { {"Result", Type::Vector4} }, "%s / %s", {"/"});
    CreateTemplateNode("Divide", functionColor, { {"A", Type::Float}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "%s / %s", {"/"});
    CreateTemplateNode("Divide", functionColor, { {"A", Type::Float}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "%s / %s", {"/"});
    CreateTemplateNode("Divide", functionColor, { {"A", Type::Float}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "%s / %s", {"/"});
    CreateTemplateNode("Divide", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "%s / %s", {"/"});
    CreateTemplateNode("Divide", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "%s / %s", {"/"});
    CreateTemplateNode("Divide", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "%s / %s", {"/"});

    CreateTemplateNode("Power", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "pow(%s, %s)", {"**"});
    CreateTemplateNode("Power", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "pow(%s, %s)", {"**"});
    CreateTemplateNode("Power", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "pow(%s, %s)", {"**"});
    CreateTemplateNode("Power", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "pow(%s, %s)", {"**"});

    CreateTemplateNode("Sqrt", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "sqrt(%s)");
    CreateTemplateNode("Sqrt", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "sqrt(%s)");
    CreateTemplateNode("Sqrt", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "sqrt(%s)");
    CreateTemplateNode("Sqrt", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "sqrt(%s)");

    CreateTemplateNode("InverseSqrt", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "inversesqrt(%s)");
    CreateTemplateNode("InverseSqrt", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "inversesqrt(%s)");
    CreateTemplateNode("InverseSqrt", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "inversesqrt(%s)");
    CreateTemplateNode("InverseSqrt", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "inversesqrt(%s)");
    
    CreateTemplateNode("Exp", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "exp(%s)");
    CreateTemplateNode("Exp", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "exp(%s)");
    CreateTemplateNode("Exp", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "exp(%s)");
    CreateTemplateNode("Exp", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "exp(%s)");
    
    CreateTemplateNode("Exp2", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "exp2(%s)");
    CreateTemplateNode("Exp2", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "exp2(%s)");
    CreateTemplateNode("Exp2", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "exp2(%s)");
    CreateTemplateNode("Exp2", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "exp2(%s)");
    
    CreateTemplateNode("Log", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "log(%s)");
    CreateTemplateNode("Log", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "log(%s)");
    CreateTemplateNode("Log", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "log(%s)");
    CreateTemplateNode("Log", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "log(%s)");

    CreateTemplateNode("Log2", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "log2(%s)");
    CreateTemplateNode("Log2", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "log2(%s)");
    CreateTemplateNode("Log2", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "log2(%s)");
    CreateTemplateNode("Log2", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "log2(%s)");
    
    CreateTemplateNode("Sin", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "sin(%s)");
    CreateTemplateNode("Sin", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "sin(%s)");
    CreateTemplateNode("Sin", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "sin(%s)");
    CreateTemplateNode("Sin", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "sin(%s)");

    CreateTemplateNode("Cos", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "cos(%s)");
    CreateTemplateNode("Cos", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "cos(%s)");
    CreateTemplateNode("Cos", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "cos(%s)");
    CreateTemplateNode("Cos", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "cos(%s)");

    CreateTemplateNode("Tan", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "tan(%s)");
    CreateTemplateNode("Tan", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "tan(%s)");
    CreateTemplateNode("Tan", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "tan(%s)");
    CreateTemplateNode("Tan", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "tan(%s)");

    CreateTemplateNode("Asin", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "asin(%s)");
    CreateTemplateNode("Asin", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "asin(%s)");
    CreateTemplateNode("Asin", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "asin(%s)");
    CreateTemplateNode("Asin", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "asin(%s)");

    CreateTemplateNode("Acos", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "acos(%s)");
    CreateTemplateNode("Acos", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "acos(%s)");
    CreateTemplateNode("Acos", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "acos(%s)");
    CreateTemplateNode("Acos", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "acos(%s)");

    CreateTemplateNode("Atan", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "atan(%s)");
    CreateTemplateNode("Atan", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "atan(%s)");
    CreateTemplateNode("Atan", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "atan(%s)");
    CreateTemplateNode("Atan", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "atan(%s)");

    CreateTemplateNode("Atan2", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "atan(%s, %s)");
    CreateTemplateNode("Atan2", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "atan(%s, %s)");
    CreateTemplateNode("Atan2", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "atan(%s, %s)");
    CreateTemplateNode("Atan2", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "atan(%s, %s)");
    
    CreateTemplateNode("Abs", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "abs(%s)");
    CreateTemplateNode("Abs", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "abs(%s)");
    CreateTemplateNode("Abs", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "abs(%s)");
    CreateTemplateNode("Abs", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "abs(%s)");

    CreateTemplateNode("Floor", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "floor(%s)");
    CreateTemplateNode("Floor", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "floor(%s)");
    CreateTemplateNode("Floor", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "floor(%s)");

    CreateTemplateNode("Ceil", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "ceil(%s)");
    CreateTemplateNode("Ceil", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "ceil(%s)");
    CreateTemplateNode("Ceil", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "ceil(%s)");

    CreateTemplateNode("Round", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "round(%s)");
    CreateTemplateNode("Round", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "round(%s)");
    CreateTemplateNode("Round", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "round(%s)");

    CreateTemplateNode("Round Even", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "roundEven(%s)");
    CreateTemplateNode("Round Even", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "roundEven(%s)");
    CreateTemplateNode("Round Even", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "roundEven(%s)");

    CreateTemplateNode("Sign", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "sign(%s)");
    CreateTemplateNode("Sign", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "sign(%s)");
    CreateTemplateNode("Sign", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "sign(%s)");
    CreateTemplateNode("Sign", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "sign(%s)");
    
    CreateTemplateNode("Fract", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "fract(%s)");
    CreateTemplateNode("Fract", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "fract(%s)");
    CreateTemplateNode("Fract", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "fract(%s)");
    CreateTemplateNode("Fract", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "fract(%s)");

    CreateTemplateNode("Saturate", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "clamp(%s, 0.0, 1.0)");
    CreateTemplateNode("Saturate", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "clamp(%s, 0.0, 1.0)");
    CreateTemplateNode("Saturate", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "clamp(%s, 0.0, 1.0)");
    CreateTemplateNode("Saturate", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "clamp(%s, 0.0, 1.0)");

    CreateTemplateNode("Reflect", functionColor, { {"I", Type::Vector3}, {"N", Type::Vector3} }, { {"Result", Type::Vector3} }, "reflect(%s, %s)");
    CreateTemplateNode("Reflect", functionColor, { {"I", Type::Vector2}, {"N", Type::Vector2} }, { {"Result", Type::Vector2} }, "reflect(%s, %s)");
    CreateTemplateNode("Reflect", functionColor, { {"I", Type::Vector4}, {"N", Type::Vector4} }, { {"Result", Type::Vector4} }, "reflect(%s, %s)");

    CreateTemplateNode("Step", functionColor, { {"Edge", Type::Float}, {"X", Type::Float} }, { {"Result", Type::Float} }, "step(%s, %s)");
    CreateTemplateNode("Step", functionColor, { {"Edge", Type::Vector2}, {"X", Type::Vector2} }, { {"Result", Type::Vector2} }, "step(%s, %s)");
    CreateTemplateNode("Step", functionColor, { {"Edge", Type::Vector3}, {"X", Type::Vector3} }, { {"Result", Type::Vector3} }, "step(%s, %s)");
    CreateTemplateNode("Step", functionColor, { {"Edge", Type::Vector4}, {"X", Type::Vector4} }, { {"Result", Type::Vector4} }, "step(%s, %s)");
    
    CreateTemplateNode("Smooth Step", functionColor, { {"Edge0", Type::Float}, {"Edge1", Type::Float}, {"X", Type::Float} }, { {"Result", Type::Float} }, "smoothstep(%s, %s, %s)");
    CreateTemplateNode("Smooth Step", functionColor, { {"Edge0", Type::Vector2}, {"Edge1", Type::Vector2}, {"X", Type::Vector2} }, { {"Result", Type::Vector2} }, "smoothstep(%s, %s, %s)");
    CreateTemplateNode("Smooth Step", functionColor, { {"Edge0", Type::Vector3}, {"Edge1", Type::Vector3}, {"X", Type::Vector3} }, { {"Result", Type::Vector3} }, "smoothstep(%s, %s, %s)");
    CreateTemplateNode("Smooth Step", functionColor, { {"Edge0", Type::Vector4}, {"Edge1", Type::Vector4}, {"X", Type::Vector4} }, { {"Result", Type::Vector4} }, "smoothstep(%s, %s, %s)");

    CreateTemplateNode("Truncate", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "trunc(%s)");
    CreateTemplateNode("Truncate", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "trunc(%s)");
    CreateTemplateNode("Truncate", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "trunc(%s)");
    CreateTemplateNode("Truncate", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "trunc(%s)");
    
    CreateTemplateNode("Min", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "min(%s, %s)");
    CreateTemplateNode("Min", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "min(%s, %s)");
    CreateTemplateNode("Min", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "min(%s, %s)");
    CreateTemplateNode("Min", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "min(%s, %s)");

    CreateTemplateNode("Max", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "max(%s, %s)");
    CreateTemplateNode("Max", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "max(%s, %s)");
    CreateTemplateNode("Max", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "max(%s, %s)");
    CreateTemplateNode("Max", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "max(%s, %s)");

    CreateTemplateNode("Clamp", functionColor, { {"A", Type::Float}, {"Min", Type::Float}, {"Max", Type::Float} }, { {"Result", Type::Float} }, "clamp(%s, %s, %s)");
    CreateTemplateNode("Clamp", functionColor, { {"A", Type::Vector2}, {"Min", Type::Vector2}, {"Max", Type::Vector2} }, { {"Result", Type::Vector2} }, "clamp(%s, %s, %s)");
    CreateTemplateNode("Clamp", functionColor, { {"A", Type::Vector3}, {"Min", Type::Vector3}, {"Max", Type::Vector3} }, { {"Result", Type::Vector3} }, "clamp(%s, %s, %s)");
    CreateTemplateNode("Clamp", functionColor, { {"A", Type::Vector4}, {"Min", Type::Vector4}, {"Max", Type::Vector4} }, { {"Result", Type::Vector4} }, "clamp(%s, %s, %s)");

    CreateTemplateNode("Mix", functionColor, { {"A", Type::Float}, {"B", Type::Float}, {"Alpha", Type::Float} }, { {"Result", Type::Float} }, "mix(%s, %s, %s)");
    CreateTemplateNode("Mix", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2}, {"Alpha", Type::Float} }, { {"Result", Type::Vector2} }, "mix(%s, %s, %s)");
    CreateTemplateNode("Mix", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3}, {"Alpha", Type::Float} }, { {"Result", Type::Vector3} }, "mix(%s, %s, %s)");
    CreateTemplateNode("Mix", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4}, {"Alpha", Type::Float} }, { {"Result", Type::Vector4} }, "mix(%s, %s, %s)");
    
    CreateTemplateNode("Modulo", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "mod(%s, %s)");
    CreateTemplateNode("Modulo", functionColor, { {"A", Type::Vector2}, {"B", Type::Float} }, { {"Result", Type::Vector2} }, "mod(%s, %s)");
    CreateTemplateNode("Modulo", functionColor, { {"A", Type::Vector3}, {"B", Type::Float} }, { {"Result", Type::Vector3} }, "mod(%s, %s)");
    CreateTemplateNode("Modulo", functionColor, { {"A", Type::Vector4}, {"B", Type::Float} }, { {"Result", Type::Vector4} }, "mod(%s, %s)");
    
    CreateTemplateNode("Dot", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Float} }, "dot(%s, %s)");
    CreateTemplateNode("Dot", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Float} }, "dot(%s, %s)");
    CreateTemplateNode("Dot", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Float} }, "dot(%s, %s)");

    CreateTemplateNode("Cross", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "cross(%s, %s)");

    CreateTemplateNode("Normalize", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "normalize(%s)");
    CreateTemplateNode("Normalize", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "normalize(%s)");
    CreateTemplateNode("Normalize", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "normalize(%s)");

    CreateTemplateNode("Length", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Float} }, "length(%s)");
    CreateTemplateNode("Length", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Float} }, "length(%s)");
    CreateTemplateNode("Length", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Float} }, "length(%s)");

    CreateTemplateNode("Distance", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Float} }, "distance(%s, %s)");
    CreateTemplateNode("Distance", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Float} }, "distance(%s, %s)");
    CreateTemplateNode("Distance", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Float} }, "distance(%s, %s)");
    
    CreateTemplateNode("Make Vector2", makeColor, { {"X", Type::Float}, {"Y", Type::Float} }, { {"Result", Type::Vector2} }, "vec2(%s, %s)");
    CreateTemplateNode("Make Vector3", makeColor, { {"X", Type::Float}, {"Y", Type::Float}, {"Z", Type::Float} }, { {"Result", Type::Vector3} }, "vec3(%s, %s, %s)");
    CreateTemplateNode("Make Vector4", makeColor, { {"X", Type::Float}, {"Y", Type::Float}, {"Z", Type::Float}, {"W", Type::Float} }, { {"Result", Type::Vector4} }, "vec4(%s, %s, %s, %s)");

    CreateTemplateNode("Break Vector2", breakColor, { {"A", Type::Vector2} }, { {"X", Type::Float}, {"Y", Type::Float} }, std::vector<std::string>{"%s.x", "%s.y"});
    CreateTemplateNode("Break Vector3", breakColor, { {"A", Type::Vector3} }, { {"X", Type::Float}, {"Y", Type::Float}, {"Z", Type::Float} }, std::vector<std::string>{"%s.x", "%s.y", "%s.z"});
    CreateTemplateNode("Break Vector4", breakColor, { {"A", Type::Vector4} }, { {"X", Type::Float}, {"Y", Type::Float}, {"Z", Type::Float}, {"W", Type::Float} }, std::vector<std::string>{"%s.x", "%s.y", "%s.z", "%s.w"});
    
    CreateTemplateNode("To Vector2", functionColor, { {"A", Type::Float} }, { {"Result", Type::Vector2} }, "vec2(%s)");
    CreateTemplateNode("To Vector2", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector2} }, "vec2(%s.xy)");
    CreateTemplateNode("To Vector2", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector2} }, "vec2(%s.xy)");

    CreateTemplateNode("To Vector3", functionColor, { {"A", Type::Float} }, { {"Result", Type::Vector3} }, "vec3(%s)");
    CreateTemplateNode("To Vector3", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector3} }, "vec3(%s, 0.0)");
    CreateTemplateNode("To Vector3", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector3} }, "vec3(%s.xyz)");

    CreateTemplateNode("To Vector4", functionColor, { {"A", Type::Float} }, { {"Result", Type::Vector4} }, "vec4(%s)");
    CreateTemplateNode("To Vector4", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector4} }, "vec4(%s, 0.0, 0.0)");
    CreateTemplateNode("To Vector4", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector4} }, "vec4(%s, 1.0)");

    CreateTemplateNode("If", otherNodeColor, {{"IsA", Type::Bool}, {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "%s ? %s : %s");
    CreateTemplateNode("If", otherNodeColor, {{"IsA", Type::Bool}, {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "%s ? %s : %s");
    CreateTemplateNode("If", otherNodeColor, {{"IsA", Type::Bool}, {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "%s ? %s : %s");
    CreateTemplateNode("If", otherNodeColor, {{"IsA", Type::Bool}, {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "%s ? %s : %s");

    CreateTemplateNode("Equal", otherNodeColor, {{"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Bool} }, "%s == %s", {"=="});
    CreateTemplateNode("Equal", otherNodeColor, {{"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Bool} }, "%s == %s", {"=="});
    CreateTemplateNode("Equal", otherNodeColor, {{"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Bool} }, "%s == %s", {"=="});
    CreateTemplateNode("Equal", otherNodeColor, {{"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Bool} }, "%s == %s", {"=="});

    CreateTemplateNode("Not", otherNodeColor, {{"A", Type::Bool} }, { {"Result", Type::Bool} }, "!%s");

    CreateTemplateNode("Not Equal", otherNodeColor, {{"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Bool} }, "%s != %s", {"!="});
    CreateTemplateNode("Not Equal", otherNodeColor, {{"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Bool} }, "%s != %s", {"!="});
    CreateTemplateNode("Not Equal", otherNodeColor, {{"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Bool} }, "%s != %s", {"!="});
    CreateTemplateNode("Not Equal", otherNodeColor, {{"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Bool} }, "%s != %s", {"!="});

    CreateTemplateNode("Inferior", otherNodeColor, {{"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Bool} }, "%s < %s", {"<"});
    CreateTemplateNode("Inferior", otherNodeColor, {{"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Bool} }, "all(lessThan(%s, %s))", {"<"});
    CreateTemplateNode("Inferior", otherNodeColor, {{"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Bool} }, "all(lessThan(%s, %s))", {"<"});
    CreateTemplateNode("Inferior", otherNodeColor, {{"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Bool} }, "all(lessThan(%s, %s))", {"<"});
    
    CreateTemplateNode("Superior", otherNodeColor, {{"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Bool} }, "%s > %s", {">"});
    CreateTemplateNode("Superior", otherNodeColor, {{"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Bool} }, "all(greaterThan(%s, %s))", {">"});
    CreateTemplateNode("Superior", otherNodeColor, {{"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Bool} }, "all(greaterThan(%s, %s))", {">"});
    CreateTemplateNode("Superior", otherNodeColor, {{"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Bool} }, "all(greaterThan(%s, %s))", {">"});
    
    CreateTemplateNode("Inferior or Equal", otherNodeColor, {{"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Bool} }, "%s <= %s", {"<="});
    CreateTemplateNode("Inferior or Equal", otherNodeColor, {{"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Bool} }, "all(lessThanEqual(%s, %s))", {"<="});
    CreateTemplateNode("Inferior or Equal", otherNodeColor, {{"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Bool} }, "all(lessThanEqual(%s, %s))", {"<="});
    CreateTemplateNode("Inferior or Equal", otherNodeColor, {{"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Bool} }, "all(lessThanEqual(%s, %s))", {"<="});
    
    CreateTemplateNode("Superior or Equal", otherNodeColor, {{"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Bool} }, "%s >= %s", {">="});
    CreateTemplateNode("Superior or Equal", otherNodeColor, {{"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Bool} }, "all(greaterThanEqual(%s, %s))", {">="});
    CreateTemplateNode("Superior or Equal", otherNodeColor, {{"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Bool} }, "all(greaterThanEqual(%s, %s))", {">="});
    CreateTemplateNode("Superior or Equal", otherNodeColor, {{"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Bool} }, "all(greaterThanEqual(%s, %s))", {">="});
    
    CreateTemplateNode("And", otherNodeColor, {{"A", Type::Bool}, {"B", Type::Bool} }, { {"Result", Type::Bool} }, "%s && %s", {"&&"});
    CreateTemplateNode("Or", otherNodeColor, {{"A", Type::Bool}, {"B", Type::Bool} }, { {"Result", Type::Bool} }, "%s || %s", {"||"});
    CreateTemplateNode("Xor", otherNodeColor, {{"A", Type::Bool}, {"B", Type::Bool} }, { {"Result", Type::Bool} }, "%s ^^ %s", {"^^"});

#ifdef _DEBUG
    RunUnitTests();
#endif

    // Sort the list by name
    // std::ranges::sort(m_templateNodes, [](const NodeMethodInfo& a, const NodeMethodInfo& b) { return a.node->GetName() < b.node->GetName(); });
}

void NodeTemplateHandler::ComputeNodesSize()
{
    if (m_computed)
        return;
    m_computed = true;
    for (NodeMethodInfo& templateNode : m_templateNodes)
    {
        NodeRef& node = templateNode.node;
        node->ComputeNodeSize();
        node->p_computed = true;
    }
}

std::vector<std::string> NodeTemplateHandler::GetTemplateFormatStrings(TemplateID templateID, int possibilityIndex)
{
    for (auto& m_templateNode : s_instance->m_templateNodes)
    {
        if (m_templateNode.node->p_templateID == templateID)
        {
            return m_templateNode.outputFormatStrings[possibilityIndex];
        }
    }
    return {};
}

void NodeTemplateHandler::AddTemplateNode(const NodeMethodInfo& info)
{
    std::string name = info.node->GetName();
    info.node->p_templateID = TemplateIDFromString(name); 
    m_templateNodes.push_back(info);
}

TemplateID NodeTemplateHandler::TemplateIDFromString(const std::string& name)
{
    return std::hash<std::string>{}(name);
}

NodeRef NodeTemplateHandler::CreateFromTemplate(size_t templateID, NodeManager* nodeManager)
{
    for (size_t i = 0; i < s_instance->m_templateNodes.size(); i++)
    {
        NodeRef nodeToClone = s_instance->m_templateNodes[i].node;
        auto hash = nodeToClone->GetTemplateID();
        if (hash == templateID)
        {
            NodeRef node(nodeToClone->Clone());
            node->p_nodeManager = nodeManager;
            node->OnCreate();
            return node;
        }   
    }
    return nullptr;
}

NodeRef NodeTemplateHandler::CreateFromTemplateName(const std::string& name)
{
    for (size_t i = 0; i < s_instance->m_templateNodes.size(); i++)
    {
        std::string nodeName = s_instance->m_templateNodes[i].node->GetName();
        if (nodeName == name)
        {
            return NodeRef(s_instance->m_templateNodes[i].node->Clone());
        }
    }
    return nullptr;
}

void NodeTemplateHandler::RemoveTemplateNode(const std::string& name)
{
    m_templateNodes.erase(std::ranges::remove_if(m_templateNodes, [&name](const NodeMethodInfo& node) {
        return node.node->GetName() == name;
    }).begin(), m_templateNodes.end());
}

NodeMethodInfo& NodeTemplateHandler::GetFromName(const std::string& name)
{
    for (size_t i = 0; i < s_instance->m_templateNodes.size(); i++)
    {
        std::string nodeName = s_instance->m_templateNodes[i].node->GetName();
        if (nodeName == name)
        {
            return s_instance->m_templateNodes[i];
        }
    }
    assert(false);
    return s_instance->m_templateNodes[0];
}

NodeRef NodeTemplateHandler::GetNodeFromName(const std::string& name)
{
    for (size_t i = 0; i < s_instance->m_templateNodes.size(); i++)
    {
        std::string nodeName = s_instance->m_templateNodes[i].node->GetName();
        if (nodeName == name)
        {
            return s_instance->m_templateNodes[i].node;
        }
    }
    return nullptr;
}

void NodeTemplateHandler::UpdateKey(const std::string& oldName, const std::string& newName)
{
    auto it = std::ranges::find_if(s_instance->m_templateNodes, [&oldName](const NodeMethodInfo& node) {
        return node.node->GetName() == oldName;
    });
    if (it != s_instance->m_templateNodes.end())
    {
        it->node->SetName(newName);
        it->node->p_templateID = TemplateIDFromString(newName);

        if (auto reroute = std::dynamic_pointer_cast<RerouteNodeNamed>(it->node))
        {
            reroute->SetRerouteName(newName);
        }
    }
}

void NodeTemplateHandler::UpdateType(const std::string& name, Type type)
{
    auto it = std::ranges::find_if(s_instance->m_templateNodes, [&name](const NodeMethodInfo& node) {
        return node.node->GetName() == name;
    });
    if (it != s_instance->m_templateNodes.end())
    {
        if (auto rerouteNode = std::dynamic_pointer_cast<RerouteNodeNamed>(it->node))
            rerouteNode->SetType(type);
    }
}

void NodeTemplateHandler::UpdateColor(const std::string& name, uint32_t color)
{
    auto it = std::ranges::find_if(s_instance->m_templateNodes, [&name](const NodeMethodInfo& node) {
        return node.node->GetName() == name;
    });
    if (it != s_instance->m_templateNodes.end())
    {
        it->node->SetTopColor(color);
        if (auto rerouteNode = std::dynamic_pointer_cast<RerouteNodeNamed>(it->node))
            rerouteNode->SetTopColor(color);
    }
}

bool NodeTemplateHandler::DoesNameExist(const std::string& name)
{
    for (size_t i = 0; i < s_instance->m_templateNodes.size(); i++)
    {
        std::string nodeName = s_instance->m_templateNodes[i].node->GetName();
        if (nodeName == name)
        {
            return true;
        }
    }
    return false;
}

void NodeTemplateHandler::CreateTemplateNode(
    const std::string& name, const uint32_t& color,
    const std::vector<StreamNameType>& inputs,
    const std::vector<StreamNameType>& outputs,
    const std::vector<std::string>& formats, const std::vector<std::string>& searchStrings)
{
    if (DoesNameExist(name))
    {
        NodeMethodInfo& info = GetFromName(name);
        for (int i = 0; i < inputs.size(); i++)
        {
            info.node->p_inputs[i]->possibleTypes.push_back(inputs[i].type);
        }
        for (int i = 0; i < outputs.size(); i++)
        {
            info.node->p_outputs[i]->possibleTypes.push_back(outputs[i].type);
        }
        info.node->p_possiblityCount++;
        info.outputFormatStrings.push_back(formats);
    }
    else
    {
        NodeRef node = std::make_shared<Node>(name);
        node->SetTopColor(color);

        for (auto& input : inputs)
        {
            node->AddInput(input.name, input.type);
        }
        for (auto& output : outputs)
        {
            node->AddOutput(output.name, output.type);
        }
        node->p_possiblityCount = 1;
        NodeMethodInfo info = {node, formats};
        info.searchStrings = searchStrings;
        AddTemplateNode(info);
    }
}

void NodeTemplateHandler::CreateTemplateNode(const std::string& name, const uint32_t& color,
                                             const std::vector<StreamNameType>& inputs,
                                             const std::vector<StreamNameType>& outputs,
                                             const std::string& format, const std::vector<std::string>& searchStrings)
{
    CreateTemplateNode(name, color, inputs, outputs, std::vector<std::string>{format}, searchStrings);
}
