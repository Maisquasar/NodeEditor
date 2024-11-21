#include "NodeTemplateHandler.h"

#include <ranges>

#include "Node.h"

std::unique_ptr<NodeTemplateHandler> NodeTemplateHandler::s_instance;

void NodeTemplateHandler::Initialize()
{
    constexpr ImU32 endColor = IM_COL32(156, 122, 72, 255);
    constexpr ImU32 functionColor = IM_COL32(72, 122, 156, 255);
    constexpr ImU32 makeColor = IM_COL32(122, 156, 72, 255);
    constexpr ImU32 breakColor = IM_COL32(156, 72, 122, 255);
    
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


#pragma region Float
    {
        NodeRef node = std::make_shared<Node>("Make float");
        node->SetTopColor(makeColor);
        node->AddInput("Value", Type::Float);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "%s"};

        AddTemplateNode(info);
    }

    
    {
        NodeRef node = std::make_shared<Node>("Add");
        node->SetTopColor(functionColor);

        node->AddInput("A", Type::Float);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "%s + %s"};
        AddTemplateNode(info);
    }
    
    {
        NodeRef node = std::make_shared<Node>("Multiply");
        node->SetTopColor(functionColor);

        node->AddInput("A", Type::Float);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "%s * %s"};
        AddTemplateNode(info);
    }
    
    {
        NodeRef node = std::make_shared<Node>("Subtract");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Float);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "%s - %s"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Divide");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Float);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "%s / %s"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("One Minus");
        node->SetTopColor(functionColor);
        
        node->AddInput("X", Type::Float);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "1 - %s"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Frac");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Float);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "fract(%s)"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Power");
        node->SetTopColor(functionColor);
        
        node->AddInput("Base", Type::Float);
        node->AddInput("Exp", Type::Float);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "pow(%s, %s)"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Saturate");
        node->SetTopColor(makeColor);
        node->AddInput("Value", Type::Float);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "clamp(%s, 0, 1)"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Arc tangent 2");
        node->SetTopColor(makeColor);
        node->AddInput("Y", Type::Float);
        node->AddInput("X", Type::Float);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "atan(%s, %s)"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Sin");
        node->SetTopColor(makeColor);
        node->AddInput("A", Type::Float);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "sin(%s)"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Cos");
        node->SetTopColor(makeColor);
        node->AddInput("A", Type::Float);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "cos(%s)"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Smooth Step");
        node->SetTopColor(makeColor);
        node->AddInput("A", Type::Float);
        node->AddInput("B", Type::Float);
        node->AddInput("X", Type::Float);
        node->AddOutput("Result", Type::Float);
        NodeMethodInfo info = {node, "smoothstep(%s, %s, %s)"};
        AddTemplateNode(info);
    }
#pragma endregion

#pragma region Int
    {
        NodeRef node = std::make_shared<Node>("Make int");
        node->SetTopColor(makeColor);
        node->AddInput("Value", Type::Int);
        node->AddOutput("Result", Type::Int);
        NodeMethodInfo info = {node, "%s"};
        AddTemplateNode(info);
    }
#pragma endregion

#pragma region Vector2
    {
        NodeRef node = std::make_shared<Node>("Make Vector2");
        node->SetTopColor(makeColor);
        node->AddInput("X", Type::Float);
        node->AddInput("Y", Type::Float);
        node->AddOutput("Result", Type::Vector2);
        NodeMethodInfo info = {node, "vec2(%s, %s)"};
        AddTemplateNode(info);
    }

    {
        NodeRef node = std::make_shared<Node>("Break Vector2");
        node->SetTopColor(breakColor);
        node->AddInput("Value", Type::Vector2);
        node->AddOutput("X", Type::Float);
        node->AddOutput("Y", Type::Float);
        NodeMethodInfo info = {node, "%s.x", "%s.y"};
        AddTemplateNode(info);
    }

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

