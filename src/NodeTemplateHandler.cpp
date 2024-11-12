#include "NodeTemplateHandler.h"

#include "Node.h"

std::unique_ptr<NodeTemplateHandler> NodeTemplateHandler::s_instance;

void NodeTemplateHandler::Initialize()
{
    constexpr ImU32 endColor = IM_COL32(156, 122, 72, 255);
    constexpr ImU32 functionColor = IM_COL32(72, 122, 156, 255);
    {
        NodeRef node = std::make_shared<Node>("Material");
        node->SetTopColor(endColor);
        node->p_allowInteraction = false;
            
        node->AddInput("Base Color", Type::Vector3);
        node->AddInput("Metallic", Type::Float);
        node->AddInput("Specular", Type::Float);
        node->AddInput("Roughness", Type::Float);
        AddTemplateNode(node);
    }
    
    {
        NodeRef node = std::make_shared<Node>("Add");
        node->SetTopColor(functionColor);

        node->AddInput("A", Type::Float);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Float);
        AddTemplateNode(node);
    }
    
    {
        NodeRef node = std::make_shared<Node>("Multiply");
        node->SetTopColor(functionColor);

        node->AddInput("A", Type::Float);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Float);
        AddTemplateNode(node);
    }
    
    {
        NodeRef node = std::make_shared<Node>("Subtract");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Float);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Float);
        AddTemplateNode(node);
    }
    
    {
        NodeRef node = std::make_shared<Node>("Divide");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Float);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Float);
        AddTemplateNode(node);
    }

    {
        NodeRef node = std::make_shared<Node>("Add");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddInput("B", Type::Vector3);
        node->AddOutput("Result", Type::Vector3);
        AddTemplateNode(node);
    }

    {
        NodeRef node = std::make_shared<Node>("Multiply");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddInput("B", Type::Vector3);
        node->AddOutput("Result", Type::Vector3);
        AddTemplateNode(node);
    }

    {
        NodeRef node = std::make_shared<Node>("Subtract");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddInput("B", Type::Vector3);
        node->AddOutput("Result", Type::Vector3);
        AddTemplateNode(node);
    }

    {
        NodeRef node = std::make_shared<Node>("Dot");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddInput("B", Type::Vector3);
        node->AddOutput("Result", Type::Float);
        AddTemplateNode(node);
    }

    {
        NodeRef node = std::make_shared<Node>("Cross");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddInput("B", Type::Vector3);
        node->AddOutput("Result", Type::Vector3);
        AddTemplateNode(node);
    }

    {
        NodeRef node = std::make_shared<Node>("Length");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddOutput("Result", Type::Float);
        AddTemplateNode(node);
    }
    
    {
        NodeRef node = std::make_shared<Node>("Normalize");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Vector3);
        node->AddOutput("Result", Type::Vector3);
        AddTemplateNode(node);
    }

    {
        NodeRef node = std::make_shared<Node>("Max");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Float);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Float);
        AddTemplateNode(node);
    }

    {
        NodeRef node = std::make_shared<Node>("Min");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Float);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Float);
        AddTemplateNode(node);
    }

    {
        NodeRef node = std::make_shared<Node>("Pow");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Float);
        node->AddInput("B", Type::Float);
        node->AddOutput("Result", Type::Float);
        AddTemplateNode(node);
    }

    {
        NodeRef node = std::make_shared<Node>("Clamp");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Float);
        node->AddInput("Min", Type::Float);
        node->AddInput("Max", Type::Float);
        node->AddOutput("Result", Type::Float);
        AddTemplateNode(node);
    }

    {
        NodeRef node = std::make_shared<Node>("Lerp");
        node->SetTopColor(functionColor);
        
        node->AddInput("A", Type::Float);
        node->AddInput("B", Type::Float);
        node->AddInput("T", Type::Float);
        node->AddOutput("Result", Type::Float);
        AddTemplateNode(node);
    }

    // Check if there is duplicate name
    for (uint32_t i = 0; i < m_templateNodes.size(); i++)
    {
        for (uint32_t j = i + 1; j < m_templateNodes.size(); j++)
        {
            if (m_templateNodes[i]->GetName() == m_templateNodes[j]->GetName())
            {
                std::string type1 = TypeEnumToString(m_templateNodes[i]->GetOutput(0)->type);
                std::string type2 = TypeEnumToString(m_templateNodes[j]->GetOutput(0)->type);
                m_templateNodes[i]->SetName(m_templateNodes[i]->GetName() + " (" + type1 + ")");
                m_templateNodes[j]->SetName(m_templateNodes[j]->GetName() + " (" + type2 + ")");
            }
        }
    }
}

void NodeTemplateHandler::AddTemplateNode(std::shared_ptr<Node> node)
{
    node->p_templateID = m_templateNodes.size();
    m_templateNodes.push_back(node);
}

std::shared_ptr<Node> NodeTemplateHandler::CreateFromTemplate(uint32_t templateID)
{
    return std::shared_ptr<Node>(s_instance->m_templateNodes[templateID]->Clone());
}
