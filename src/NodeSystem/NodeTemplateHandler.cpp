#include "NodeSystem/NodeTemplateHandler.h"
#include "NodeSystem/Node.h"

#include <ranges>

#include "NodeSystem/CustomNode.h"
#include "NodeSystem/ParamNode.h"

#include "NodeSystem/ShaderMaker.h"
#include "Render/Framebuffer.h"

class Shader;
std::unique_ptr<NodeTemplateHandler> NodeTemplateHandler::s_instance;

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
    Ref<Shader> shader = std::make_shared<Shader>();
    if (!shader->LoadVertexShader("shaders/shader.vert"))
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
        auto stringType = ShaderMaker::TypeToGLSLType(input->type);
        std::string inputName = input->name + "_" + std::to_string(i);
        content += stringType + " " + inputName + ";\n";
        inputNames[i] = inputName;
    }

    auto formatList = node->GetFormatStrings();
    for (int k = 0; k <  node->p_outputs.size(); k++)
    {
        std::string variableName = node->p_outputs[k]->name + "_" + std::to_string(k);
        std::string glslType = ShaderMaker::TypeToGLSLType(node->p_outputs[k]->type);
            
        std::string thisContent = glslType + " " + variableName + " = ";
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
            auto parentVariableName =inputNames[j];
            toFormat += FormatString(firstHalf, parentVariableName.c_str());
        }
        thisContent += toFormat + secondHalf + ";\n";

        content += thisContent;
    }
    content += "}\n";
    bool success = shader->SetFragmentShaderContent(content);
    if (!success)
    {
        std::cout << "Failed with node: " << node->p_name << std::endl;
        return false;
    }
    
    return shader->Link();
}

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
        node->p_alwaysVisibleOnContext = true;
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
        node->p_alwaysVisibleOnContext = true;
        
        NodeMethodInfo info{node};
        AddTemplateNode(info);
    }
#pragma endregion

#pragma region Float
    CreateTemplateNode("Make float", makeColor, { {"Value", Type::Float} }, { {"Result", Type::Float} }, "%s");
    
    CreateTemplateNode("To Vector2", makeColor, { {"X", Type::Float}, {"Y", Type::Float} }, { {"Result", Type::Vector2} }, "vec2(%s)");
    CreateTemplateNode("To Vector3", makeColor, { {"X", Type::Float}}, { {"Result", Type::Vector3} }, "vec3(%s)");
    CreateTemplateNode("To Vector4", makeColor, { {"X", Type::Float}}, { {"Result", Type::Vector4} }, "vec4(%s)");
    
    CreateTemplateNode("Add", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "%s + %s", {"+"});
    CreateTemplateNode("Subtract", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "%s - %s", {"-"});
    CreateTemplateNode("Multiply", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "%s * %s", {"*"});
    CreateTemplateNode("Multiply scalar", functionColor, { {"A", Type::Vector2}, {"B", Type::Float} }, { {"Result", Type::Vector2} }, "%s * %s", {"*"});
    CreateTemplateNode("Multiply scalar", functionColor, { {"A", Type::Vector3}, {"B", Type::Float} }, { {"Result", Type::Vector3} }, "%s * %s", {"*"});
    CreateTemplateNode("Multiply scalar", functionColor, { {"A", Type::Vector4}, {"B", Type::Float} }, { {"Result", Type::Vector4} }, "%s * %s", {"*"});
    CreateTemplateNode("Divide", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "%s / %s", {"/"});
    
    CreateTemplateNode("One Minus", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "1.0 - %s", {"1- "});
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
    CreateTemplateNode("Mod", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "modf(%s, %s)");
    CreateTemplateNode("Max", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "max(%s, %s)");
    CreateTemplateNode("Min", functionColor, { {"A", Type::Float}, {"B", Type::Float} }, { {"Result", Type::Float} }, "min(%s, %s)");
    CreateTemplateNode("Clamp", functionColor, { {"Value", Type::Float}, {"Min", Type::Float}, {"Max", Type::Float} }, { {"Result", Type::Float} }, "clamp(%s, %s, %s)");
    CreateTemplateNode("Lerp", functionColor, { {"A", Type::Float}, {"B", Type::Float}, {"T", Type::Float} }, { {"Result", Type::Float} }, "mix(%s, %s, %s)");
    CreateTemplateNode("Smooth Step", functionColor, { {"Edge0", Type::Float}, {"Edge1", Type::Float}, {"Value", Type::Float} }, { {"Result", Type::Float} }, "smoothstep(%s, %s, %s)");
    CreateTemplateNode("Step", functionColor, { {"Edge", Type::Float}, {"Value", Type::Float} }, { {"Result", Type::Float} }, "step(%s, %s)");
    CreateTemplateNode("Fract", functionColor, { {"A", Type::Float} }, { {"Result", Type::Float} }, "fract(%s)");
    CreateTemplateNode("Power", functionColor, { {"Base", Type::Float}, {"Exp", Type::Float} }, { {"Result", Type::Float} }, "pow(%s, %s)", {"**"});
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
    CreateTemplateNode("Break Vector2", breakColor, { {"Value", Type::Vector2} }, { {"X", Type::Float}, {"Y", Type::Float} }, std::vector<std::string>{"%s.x", "%s.y"});

    CreateTemplateNode("To Vector3", makeColor, { {"Value", Type::Vector2} }, { {"Result", Type::Vector3} }, "vec3(%s, 0.0)");
    CreateTemplateNode("To Vector4", makeColor, { {"Value", Type::Vector2} }, { {"Result", Type::Vector4} }, "vec4(%s, 0.0, 0.0)");

    CreateTemplateNode("Add", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "%s + %s", {"+"});
    CreateTemplateNode("Subtract", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "%s - %s", {"-"});
    CreateTemplateNode("Multiply", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "%s * %s", {"*"});
    CreateTemplateNode("Divide", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "%s / %s", {"/"});
    
    CreateTemplateNode("Dot", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Float} }, "dot(%s, %s)");
    CreateTemplateNode("Normalize", functionColor, { {"Value", Type::Vector2} }, { {"Result", Type::Vector2} }, "normalize(%s)");
    CreateTemplateNode("Length", functionColor, { {"Value", Type::Vector2} }, { {"Result", Type::Float} }, "length(%s)");
    CreateTemplateNode("Distance", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Float} }, "distance(%s, %s)");
    
    CreateTemplateNode("One Minus", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "1.0 - %s", {"1-"});
    CreateTemplateNode("Abs", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "abs(%s)");
    CreateTemplateNode("Floor", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "floor(%s)");
    CreateTemplateNode("Ceil", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "ceil(%s)");
    CreateTemplateNode("Round", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "round(%s)");
    CreateTemplateNode("Sign", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "sign(%s)");
    CreateTemplateNode("Sqrt", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "sqrt(%s)");
    CreateTemplateNode("Sin", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "sin(%s)");
    CreateTemplateNode("Cos", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "cos(%s)");
    CreateTemplateNode("Tan", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "tan(%s)");
    CreateTemplateNode("ASin", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "asin(%s)");
    CreateTemplateNode("ACos", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "acos(%s)");
    CreateTemplateNode("ATan", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "atan(%s)");
    CreateTemplateNode("Mod", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "modf(%s, %s)");
    CreateTemplateNode("Max", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "max(%s, %s)");
    CreateTemplateNode("Min", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2} }, { {"Result", Type::Vector2} }, "min(%s, %s)");
    CreateTemplateNode("Clamp", functionColor, { {"Value", Type::Vector2}, {"Min", Type::Vector2}, {"Max", Type::Vector2} }, { {"Result", Type::Vector2} }, "clamp(%s, %s, %s)");
    CreateTemplateNode("Lerp", functionColor, { {"A", Type::Vector2}, {"B", Type::Vector2}, {"T", Type::Vector2} }, { {"Result", Type::Vector2} }, "mix(%s, %s, %s)");
    CreateTemplateNode("Smooth Step", functionColor, { {"Edge0", Type::Vector2}, {"Edge1", Type::Vector2}, {"Value", Type::Vector2} }, { {"Result", Type::Vector2} }, "smoothstep(%s, %s, %s)");
    CreateTemplateNode("Step", functionColor, { {"Edge", Type::Vector2}, {"Value", Type::Vector2} }, { {"Result", Type::Vector2} }, "step(%s, %s)");
    CreateTemplateNode("Fract", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "fract(%s)");
    CreateTemplateNode("Power", functionColor, { {"Base", Type::Vector2}, {"Exp", Type::Vector2} }, { {"Result", Type::Vector2} }, "pow(%s, %s)", {"**"});
    CreateTemplateNode("Log", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "log(%s)");
    CreateTemplateNode("Exp", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "exp(%s)");
    CreateTemplateNode("Log2", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "log2(%s)");
    CreateTemplateNode("Saturate", functionColor, { {"A", Type::Vector2} }, { {"Result", Type::Vector2} }, "clamp(%s, 0.0, 1.0)");
    
    
#pragma endregion

#pragma region Vector3
    CreateTemplateNode("Make Vector3", makeColor, { {"X", Type::Float}, {"Y", Type::Float}, {"Z", Type::Float} }, { {"Result", Type::Vector3} }, "vec3(%s, %s, %s)");
    CreateTemplateNode("Break Vector3", makeColor, { {"Value", Type::Vector3} }, { {"X", Type::Float}, {"Y", Type::Float}, {"Z", Type::Float} }, std::vector<std::string>{"%s.x", "%s.y", "%s.z"});
    
    CreateTemplateNode("To Vector2", makeColor, { {"Value", Type::Vector3} }, { {"Result", Type::Vector2} }, "vec2(%s)");
    CreateTemplateNode("To Vector4", makeColor, { {"Value", Type::Vector3} }, { {"Result", Type::Vector4} }, "vec4(%s, 0.0)");

    CreateTemplateNode("Add", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "%s + %s", {"+"});
    CreateTemplateNode("Subtract", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "%s - %s", {"-"});
    CreateTemplateNode("Multiply", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "%s * %s", {"*"});
    CreateTemplateNode("Divide", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "%s / %s", {"/"});
    
    CreateTemplateNode("Dot", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Float} }, "dot(%s, %s)");
    CreateTemplateNode("Cross", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "cross(%s, %s)");
    CreateTemplateNode("Normalize", functionColor, { {"Value", Type::Vector3} }, { {"Result", Type::Vector3} }, "normalize(%s)");
    CreateTemplateNode("Length", functionColor, { {"Value", Type::Vector3} }, { {"Result", Type::Float} }, "length(%s)");
    CreateTemplateNode("Distance", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Float} }, "distance(%s, %s)");
    
    CreateTemplateNode("One Minus", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "1.0 - %s", {"1-"});
    CreateTemplateNode("Abs", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "abs(%s)");
    CreateTemplateNode("Floor", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "floor(%s)");
    CreateTemplateNode("Ceil", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "ceil(%s)");
    CreateTemplateNode("Round", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "round(%s)");
    CreateTemplateNode("Sign", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "sign(%s)");
    CreateTemplateNode("Sqrt", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "sqrt(%s)");
    CreateTemplateNode("Sin", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "sin(%s)");
    CreateTemplateNode("Cos", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "cos(%s)");
    CreateTemplateNode("Tan", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "tan(%s)");
    CreateTemplateNode("ASin", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "asin(%s)");
    CreateTemplateNode("ACos", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "acos(%s)");
    CreateTemplateNode("ATan", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "atan(%s)");
    CreateTemplateNode("Mod", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "modf(%s, %s)");
    CreateTemplateNode("Max", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "max(%s, %s)");
    CreateTemplateNode("Min", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3} }, { {"Result", Type::Vector3} }, "min(%s, %s)");
    CreateTemplateNode("Clamp", functionColor, { {"Value", Type::Vector3}, {"Min", Type::Vector3}, {"Max", Type::Vector3} }, { {"Result", Type::Vector3} }, "clamp(%s, %s, %s)");
    CreateTemplateNode("Lerp", functionColor, { {"A", Type::Vector3}, {"B", Type::Vector3}, {"T", Type::Vector3} }, { {"Result", Type::Vector3} }, "mix(%s, %s, %s)");
    CreateTemplateNode("Smooth Step", functionColor, { {"Edge0", Type::Vector3}, {"Edge1", Type::Vector3}, {"Value", Type::Vector3} }, { {"Result", Type::Vector3} }, "smoothstep(%s, %s, %s)");
    CreateTemplateNode("Step", functionColor, { {"Edge", Type::Vector3}, {"Value", Type::Vector3} }, { {"Result", Type::Vector3} }, "step(%s, %s)");
    CreateTemplateNode("Fract", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "fract(%s)");
    CreateTemplateNode("Power", functionColor, { {"Base", Type::Vector3}, {"Exp", Type::Vector3} }, { {"Result", Type::Vector3} }, "pow(%s, %s)", {"**"});
    CreateTemplateNode("Log", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "log(%s)");
    CreateTemplateNode("Exp", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "exp(%s)");
    CreateTemplateNode("Log2", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "log2(%s)");
    CreateTemplateNode("Saturate", functionColor, { {"A", Type::Vector3} }, { {"Result", Type::Vector3} }, "clamp(%s, 0.0, 1.0)");

    CreateTemplateNode("Mix", functionColor, {{"A", Type::Vector3}, {"B", Type::Vector3}, {"T", Type::Float} }, { {"Result", Type::Vector3} }, "mix(%s, %s, %s)");
    
#pragma endregion

#pragma region Vector4
    CreateTemplateNode("Make Vector4", functionColor, { {"X", Type::Float}, {"Y", Type::Float}, {"Z", Type::Float}, {"W", Type::Float} }, { {"Result", Type::Vector4} }, "vec4(%s, %s, %s, %s)");
    CreateTemplateNode("Break Vector4", functionColor, { {"A", Type::Vector4} }, { {"X", Type::Float}, {"Y", Type::Float}, {"Z", Type::Float}, {"W", Type::Float} }, std::vector<std::string>{"%s.x", "%s.y", "%s.z", "%s.w"});

    CreateTemplateNode("To Vector2", makeColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector2} }, "vec2(%s)");
    CreateTemplateNode("To Vector3", makeColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector3} }, "vec3(%s)");

    CreateTemplateNode("Add", functionColor, {{"A", Type::Vector4}, {"B", Type::Vector4}}, {{"Result", Type::Vector4}}, "%s + %s", {"+"});
    CreateTemplateNode("Subtract", functionColor, {{"A", Type::Vector4}, {"B", Type::Vector4}}, {{"Result", Type::Vector4}}, "%s - %s", {"-"});
    CreateTemplateNode("Multiply", functionColor, {{"A", Type::Vector4}, {"B", Type::Vector4}}, {{"Result", Type::Vector4}}, "%s * %s", {"*"});
    CreateTemplateNode("Divide", functionColor, {{"A", Type::Vector4}, {"B", Type::Vector4}}, {{"Result", Type::Vector4}}, "%s / %s", {"/"});
    
    CreateTemplateNode("Dot", functionColor, {{"A", Type::Vector4}, {"B", Type::Vector4}}, {{"Result", Type::Float}}, "dot(%s, %s)");
    CreateTemplateNode("Normalize", functionColor, {{"A", Type::Vector4}}, {{"Result", Type::Vector4}}, "normalize(%s)");
    CreateTemplateNode("Length", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Float} }, "length(%s)");
    CreateTemplateNode("Distance", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Float} }, "distance(%s, %s)");

    CreateTemplateNode("One Minus", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "1.0 - %s", {"1-"});
    CreateTemplateNode("Abs", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "abs(%s)");
    CreateTemplateNode("Floor", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "floor(%s)");
    CreateTemplateNode("Ceil", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "ceil(%s)");
    CreateTemplateNode("Round", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "round(%s)");
    CreateTemplateNode("Sign", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "sign(%s)");
    CreateTemplateNode("Sqrt", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "sqrt(%s)");
    CreateTemplateNode("Sin", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "sin(%s)");
    CreateTemplateNode("Cos", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "cos(%s)");
    CreateTemplateNode("Tan", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "tan(%s)");
    CreateTemplateNode("ASin", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "asin(%s)");
    CreateTemplateNode("ACos", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "acos(%s)");
    CreateTemplateNode("ATan", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "atan(%s)");
    CreateTemplateNode("Mod", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "modf(%s, %s)");
    CreateTemplateNode("Max", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "max(%s, %s)");
    CreateTemplateNode("Min", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4} }, { {"Result", Type::Vector4} }, "min(%s, %s)");
    CreateTemplateNode("Clamp", functionColor, { {"Value", Type::Vector4}, {"Min", Type::Vector4}, {"Max", Type::Vector4} }, { {"Result", Type::Vector4} }, "clamp(%s, %s, %s)");
    CreateTemplateNode("Lerp", functionColor, { {"A", Type::Vector4}, {"B", Type::Vector4}, {"T", Type::Vector4} }, { {"Result", Type::Vector4} }, "mix(%s, %s, %s)");
    CreateTemplateNode("Smooth Step", functionColor, { {"Edge0", Type::Vector4}, {"Edge1", Type::Vector4}, {"Value", Type::Vector4} }, { {"Result", Type::Vector4} }, "smoothstep(%s, %s, %s)");
    CreateTemplateNode("Step", functionColor, { {"Edge", Type::Vector4}, {"Value", Type::Vector4} }, { {"Result", Type::Vector4} }, "step(%s, %s)");
    CreateTemplateNode("Fract", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "fract(%s)");
    CreateTemplateNode("Power", functionColor, { {"Base", Type::Vector4}, {"Exp", Type::Vector4} }, { {"Result", Type::Vector4} }, "pow(%s, %s)", {"**"});
    CreateTemplateNode("Log", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "log(%s)");
    CreateTemplateNode("Exp", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "exp(%s)");
    CreateTemplateNode("Log2", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "log2(%s)");
    CreateTemplateNode("Saturate", functionColor, { {"A", Type::Vector4} }, { {"Result", Type::Vector4} }, "clamp(%s, 0.0, 1.0)");
#pragma endregion
    
    // Check if there is duplicate name
    for (uint32_t i = 0; i < m_templateNodes.size(); i++)
    {
        for (uint32_t j = i + 1; j < m_templateNodes.size(); j++)
        {
            if (m_templateNodes[i].node->GetName() == m_templateNodes[j].node->GetName())
            {
                std::string type1 = TypeEnumToString(m_templateNodes[i].node->GetInput(0)->type);
                std::string type2 = TypeEnumToString(m_templateNodes[j].node->GetInput(0)->type);
                m_templateNodes[i].node->SetName(m_templateNodes[i].node->GetName() + " (" + type1 + ")");
                m_templateNodes[i].node->p_templateID = TemplateIDFromString(m_templateNodes[i].node->GetName());
                m_templateNodes[j].node->SetName(m_templateNodes[j].node->GetName() + " (" + type2 + ")");
                m_templateNodes[j].node->p_templateID = TemplateIDFromString(m_templateNodes[j].node->GetName());
            }
        }
    }
    
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

void NodeTemplateHandler::CreateTemplateNode(
    const std::string& name, const uint32_t& color,
    const std::vector<std::tuple<std::string, Type>>& inputs,
    const std::vector<std::tuple<std::string, Type>>& outputs,
    const std::vector<std::string>& formats, const std::vector<std::string>& searchStrings)
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
    info.searchStrings = std::move(searchStrings);
    AddTemplateNode(info);
}

void NodeTemplateHandler::CreateTemplateNode(const std::string& name, const uint32_t& color,
    const std::vector<std::tuple<std::string, Type>>& inputs, const std::vector<std::tuple<std::string, Type>>& outputs,
    const std::string& format, const std::vector<std::string>& searchStrings)
{
    CreateTemplateNode(name, color, inputs, outputs, std::vector<std::string>{format}, searchStrings);
}
