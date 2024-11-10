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
}

void NodeTemplateHandler::AddTemplateNode(std::shared_ptr<Node> node)
{
    node->p_templateID = m_templateNodes.size();
    m_templateNodes.push_back(node);
}

std::shared_ptr<Node> NodeTemplateHandler::CreateFromTemplate(uint32_t templateID) const
{
    return std::shared_ptr<Node>(m_templateNodes[templateID]->Clone());
}
