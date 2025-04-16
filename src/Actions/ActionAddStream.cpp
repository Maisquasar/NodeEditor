#include <utility>

#include "Actions/ActionAddStream.h"

ActionAddStream::ActionAddStream(CustomNode* node, const std::string& name, Type type, StreamType streamType)
{
    m_node = node;
    m_name = name;
    m_type = type;
    m_streamType = streamType;
}

void ActionAddStream::Do()
{
    if (m_streamType == StreamType::Input)
        m_node->AddInput(m_name, m_type);
    else
        m_node->AddOutput(m_name, m_type);
    m_node->UpdateFunction();
}

void ActionAddStream::Undo()
{
    // Remove Last added stream
    if (m_streamType == StreamType::Input)
    {
        auto input = m_node->GetInputs().back();
        m_node->GetNodeManager()->GetLinkManager()->RemoveLink(input);
        m_node->RemoveInput(input->index);
    }
    else
    {
        auto output = m_node->GetOutputs().back();
        m_node->GetNodeManager()->GetLinkManager()->RemoveLinks(output);
        m_node->RemoveOutput(output->index);
    }
    m_node->UpdateFunction();
}

std::unordered_set<UUID> ActionAddStream::NodeToUpdate() const
{
    return { m_node->GetUUID() };
}

ActionRemoveStream::ActionRemoveStream(CustomNode* node, StreamRef stream)
{
    m_node = node;
    m_stream = std::move(stream);

    if (m_stream->streamType == StreamType::Input)
    {
        auto linkRef = m_node->GetNodeManager()->GetLinkManager()->GetLinkWithInput(m_stream->parentUUID, m_stream->index);
        if (linkRef)
        {
            m_prevLinks.push_back(*linkRef);
        }
    }
    else
    {
        auto outputLinks = m_node->GetNodeManager()->GetLinkManager()->GetLinksWithOutput(m_stream->parentUUID, m_stream->index);
        std::ranges::for_each(outputLinks, [&](const LinkRef& link)
        {
            m_prevLinks.push_back(*link);
        });
    }
}

void ActionRemoveStream::Do()
{
    if (m_stream->streamType == StreamType::Input)
    {
        m_node->GetNodeManager()->GetLinkManager()->RemoveLink(std::dynamic_pointer_cast<Input>(m_stream));
        m_node->RemoveInput(m_stream->index);
    }
    else
    {
        m_node->GetNodeManager()->GetLinkManager()->RemoveLinks(std::dynamic_pointer_cast<Output>(m_stream));
        m_node->RemoveOutput(m_stream->index);
    }
    m_node->UpdateFunction();
}

void ActionRemoveStream::Undo()
{
    if (m_stream->streamType == StreamType::Input)
    {
        m_node->AddInput(m_stream->name, m_stream->type);
        InputRef input = m_node->GetInput(m_stream->index);
        input->value = std::dynamic_pointer_cast<Input>(m_stream)->GetValue();
    }
    else
    {
        m_node->AddOutput(m_stream->name, m_stream->type);
        OutputRef output = m_node->GetOutput(m_stream->index);
    }

    for (auto& link : m_prevLinks)
    {
        m_node->GetNodeManager()->GetLinkManager()->AddLink(link);
    }
    m_node->UpdateFunction();
}

std::unordered_set<UUID> ActionRemoveStream::NodeToUpdate() const
{
    return { m_node->GetUUID() };
}

ActionChangeStreamType::ActionChangeStreamType(Node* node, int index) : m_node(node), m_index(index)
{
    m_prevIndex = node->GetCurrentPossibility();

    auto linkManager = node->GetNodeManager()->GetLinkManager();
    for (auto& input : node->GetInputs())
    {
        auto link = linkManager->GetLinkWithInput(node->GetUUID(), input->index);
        if (link)
        {
            m_prevLinks.push_back(*link);
        }
    }

    for (auto& output : node->GetOutputs())
    {
        auto links = linkManager->GetLinksWithOutput(node->GetUUID(), output->index);
        std::ranges::for_each(links, [&](const LinkRef& link)
        {
            m_prevLinks.push_back(*link);
        });
    }
}

void ActionChangeStreamType::Do()
{
    m_node->ConvertStream(m_index);
}

void ActionChangeStreamType::Undo()
{
    m_node->ConvertStream(m_prevIndex);

    auto linkManager = m_node->GetNodeManager()->GetLinkManager();
    for (auto& link : m_prevLinks)
    {
        if (linkManager->LinkExists(link))
            continue;
        linkManager->AddLink(link);
    }
}

std::unordered_set<UUID> ActionChangeStreamType::NodeToUpdate() const
{
    std::unordered_set<UUID> nodesToUpdate;
    for (auto& link : m_prevLinks)
    {
        nodesToUpdate.insert(link.toNodeIndex);
    }
    return { nodesToUpdate };
}