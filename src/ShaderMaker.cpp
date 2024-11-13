#include "ShaderMaker.h"

#include "NodeTemplateHandler.h"

template<typename ... Args>
std::string FormatString( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

void ShaderMaker::RecurrenceWork(NodeManager* manager, const NodeRef& endNode, TemplateList& templateList, std::string& content,
                         LinkManager* linkManager)
{
    for (int i = endNode->p_inputs.size() - 1; i >= 0; i--)
    {
        LinkRef link = linkManager->GetLinkLinkedToInput(endNode->GetUUID(), i).lock();
        if (link == nullptr)
            continue;
        NodeRef node = manager->GetNode(link->fromNodeIndex).lock();
        if (node == nullptr)
            continue;
        RecurrenceWork(manager, node, templateList, content, linkManager);
        NodeMethodInfo templateNode = templateList[node->p_templateID];
        std::string variableName = "_" + std::to_string(node->p_uuid);
        content += FormatString("float %s = ", variableName.c_str());
        std::string toFormat = templateNode.formatString;
        std::string secondHalf = toFormat;
        toFormat.clear();
        for (int j = 0; j < node->p_inputs.size(); j++)
        {
            size_t index = secondHalf.find_first_of("%") + 2;
            if (index == std::string::npos)
                break;
            std::string firstHalf = secondHalf.substr(0, index);
            secondHalf = secondHalf.substr(index);
            if (!templateNode.isMaker && !m_variablesNames.empty())
            {
                toFormat += FormatString(firstHalf, m_variablesNames.back().c_str());
                m_variablesNames.erase(m_variablesNames.end() - 1);
            }
            else if (templateNode.isMaker)
            {
                float value = 0.f;
                toFormat += FormatString(firstHalf, value);
            }
        }
        content += toFormat + "\n";
        m_variablesNames.push_back(variableName);
    }
}

void ShaderMaker::CreateFragmentShader(NodeManager* manager)
{
    // Get all nodes connected to the end node
    NodeRef endNode = manager->GetNodeWithTemplate(0).lock();

    TemplateList& templateList = NodeTemplateHandler::GetInstance()->GetTemplates();
    
    std::string content;
    auto linkManager = manager->GetLinkManager();

    RecurrenceWork(manager, endNode, templateList, content, linkManager);
    // TODO
    ImGui::SetClipboardText(content.c_str());

    std::cout << content;
}
