#include <utility>

#include "Actions/ActionChangeType.h"

#include "NodeSystem/CustomNode.h"
#include "NodeSystem/ParamNode.h"
#include "NodeSystem/RerouteNodeNamed.h"

ActionChangeTypeParam::ActionChangeTypeParam(ParamNodeManager* paramNodeManager, const std::string& paramName,
                                             Type type, Type oldType) 
{
    m_paramNodeManager = paramNodeManager;
    m_paramName = paramName;
    m_type = type;
    m_oldType = oldType;

    auto nodes = m_paramNodeManager->GetParamNodes(paramName);
    for (auto& node : nodes)
    {
        std::vector<LinkRef> outputLinks = node->GetNodeManager()->GetLinkManager()->GetLinksWithOutput(node->GetOutput(0));
        LinkRef inputLink = node->GetNodeManager()->GetLinkManager()->GetLinkWithInput(node->GetUUID(), 0);
        
        std::ranges::for_each(outputLinks, [&](const LinkRef& link)
        {
            m_prevLinks.push_back(*link);
        });
        if (inputLink)
        {
            m_prevLinks.push_back(*inputLink);
        }
    }
}

void ActionChangeTypeParam::Do()
{
    m_paramNodeManager->OnUpdateType(m_paramName);
    m_paramNodeManager->UpdateType(m_paramName, m_type);
}

void ActionChangeTypeParam::Undo()
{
    // Don't need to update links
    m_paramNodeManager->UpdateType(m_paramName, m_oldType);

    for (Link& link : m_prevLinks)
    {
        m_paramNodeManager->GetNodeManager()->GetLinkManager()->AddLink(link);
    }
}

std::unordered_set<UUID> ActionChangeTypeParam::NodeToUpdate() const
{
    auto paramNodes = m_paramNodeManager->GetParamNodes(m_paramName);
    std::unordered_set<UUID> uuids;
    for (auto& node : paramNodes)
    {
        uuids.insert(node->GetUUID());
    }
    return uuids;
}

ActionChangeTypeCustom::ActionChangeTypeCustom(CustomNode* node, StreamRef stream, Type type, Type oldType)
{
    m_customNode = node;
    m_stream = std::move(stream);
    m_type = type;
    m_oldType = oldType;

    if (m_stream->streamType == StreamType::Input)
    {
        const LinkRef linkRef = m_customNode->GetNodeManager()->GetLinkManager()->GetLinkWithInput(m_stream->parentUUID, m_stream->index);
        if (linkRef)
            m_prevLinks.push_back(*linkRef);
    }
    else
    {
        std::vector<LinkRef> outputLinks = m_customNode->GetNodeManager()->GetLinkManager()->GetLinksWithOutput(m_stream->parentUUID, m_stream->index);
        std::ranges::for_each(outputLinks, [&](const LinkRef& link)
        {
            m_prevLinks.push_back(*link);
        });
    }
}

void ActionChangeTypeCustom::Do()
{
    if (m_stream->streamType == StreamType::Input)
    {
        m_customNode->ChangeInputType(m_stream->index, m_type);
        m_customNode->GetNodeManager()->GetLinkManager()->RemoveLink(std::dynamic_pointer_cast<Input>(m_stream));
    }
    else
    {
        m_customNode->ChangeOutputType(m_stream->index, m_type);
        m_customNode->GetNodeManager()->GetLinkManager()->RemoveLinks(std::dynamic_pointer_cast<Output>(m_stream));
    }
    m_customNode->UpdateFunction();
}

void ActionChangeTypeCustom::Undo()
{
    if (m_stream->streamType == StreamType::Input)
    {
        m_customNode->ChangeInputType(m_stream->index, m_oldType);
    }
    else
    {
        m_customNode->ChangeOutputType(m_stream->index, m_oldType);
    }
    for (auto& link : m_prevLinks)
    {
        m_customNode->GetNodeManager()->GetLinkManager()->AddLink(link);
    }
    m_customNode->UpdateFunction();
}

std::unordered_set<UUID> ActionChangeTypeCustom::NodeToUpdate() const
{
    return { m_stream->parentUUID };
}

ActionChangeTypeRerouteNode::ActionChangeTypeRerouteNode(RerouteNodeNamedManager* rerouteNodeManager,
                                                         const std::string& name, Type type, Type oldType)
{
    m_rerouteNodeNamedManager = rerouteNodeManager;
    m_name = name;
    m_type = type;
    m_oldType = oldType;

    const RerouteNodeNamedData& rerouteNodeNamedData = rerouteNodeManager->GetNodeData(name);
    for (auto& node : rerouteNodeNamedData.node)
    {
        if (node->IsDefinition())
        {
            if (Ref inputLink = node->GetNodeManager()->GetLinkManager()->GetLinkWithInput(node->GetUUID(), 0))
            {
                m_prevLinks.push_back(*inputLink);
            }
        }
        std::vector links = node->GetNodeManager()->GetLinkManager()->GetLinksWithOutput(node->GetOutput(0));
        std::ranges::for_each(links, [&](const LinkRef& link)
        {
            m_prevLinks.push_back(*link);
        });
    }
}

void ActionChangeTypeRerouteNode::Do()
{
    RerouteNodeNamed* defNode = m_rerouteNodeNamedManager->GetDefinitionNode(m_name);
    m_rerouteNodeNamedManager->UpdateType(m_name, m_type);

    for (auto& link : m_prevLinks)
    {
        defNode->GetNodeManager()->GetLinkManager()->RemoveLink(link);
    }
}

void ActionChangeTypeRerouteNode::Undo()
{
    RerouteNodeNamed* defNode = m_rerouteNodeNamedManager->GetDefinitionNode(m_name);
    m_rerouteNodeNamedManager->UpdateType(m_name, m_oldType);
    for (auto& link : m_prevLinks)
    {
        defNode->GetNodeManager()->GetLinkManager()->AddLink(link);
    }
}

std::unordered_set<UUID> ActionChangeTypeRerouteNode::NodeToUpdate() const
{
    RerouteNodeNamed* rerouteNodeNamed = m_rerouteNodeNamedManager->GetDefinitionNode(m_name);
    if (!rerouteNodeNamed)
        return {};
    return { rerouteNodeNamed->GetUUID() };
}
