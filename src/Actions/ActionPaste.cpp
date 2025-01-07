#include "Actions/ActionPaste.h"

#include <CppSerializer.h>
#include <map>

#include "NodeWindow.h"

void ActionPaste::Do()
{
    LinkManager* linkManager = m_nodeManager->GetLinkManager();
    std::string clipboardText = m_clipboardText;
    CppSer::Parser parser = CppSer::Parser(clipboardText);
        
    SerializedData data = m_nodeManager->DeserializeData(parser);

    // Calculate the bounding box of copied nodes
    Vec2f minPos(FLT_MAX, FLT_MAX);
    Vec2f maxPos(-FLT_MAX, -FLT_MAX);
    for (const auto& node : data.nodes)
    {
        // Adjust for zoom and origin when calculating node bounds
        Vec2f min = node->GetMin(m_zoom, m_origin);
        Vec2f max = node->GetMax(min, m_zoom);
        minPos.x = std::min(minPos.x, min.x);
        minPos.y = std::min(minPos.y, min.y);
        maxPos.x = std::max(maxPos.x, max.x);
        maxPos.y = std::max(maxPos.y, max.y);
    }
        
    // Calculate the local center of the copied nodes
    Vec2f localCenter = (minPos + maxPos) * 0.5f;
        
    // Get the mouse position as the paste position, adjusted for zoom and origin
    Vec2f mousePos = m_mousePos;
    Vec2f pastePosition = NodeManager::ToGrid(mousePos, m_zoom, m_origin);
        
    // Map old UUIDs to new UUIDs after resetting them
    std::map<UUID, UUID> uuidMap;
    for (auto& node : data.nodes)
    {
        UUID oldUUID = node->GetUUID();
        node->ResetUUID();
        uuidMap[oldUUID] = node->GetUUID();

        for (InputRef& input : node->GetInputs())
        {
            input->parentUUID = node->GetUUID();
        }
        for (OutputRef& output : node->GetOutputs())
        {
            output->parentUUID = node->GetUUID();
        }
            
        // Adjust node position relative to the local center and paste position, taking zoom and origin into account
        Vec2f offset = (node->GetPosition() - NodeManager::ToGrid(localCenter, m_zoom, m_origin));
        node->SetPosition(pastePosition + offset);
            
        // Add node to node manager
        m_nodeManager->AddNode(node);

        m_pastedNodes.push_back(node);
    }

    // Adjust link references to new node UUIDs
    for (auto& link : data.links)
    {
        if (uuidMap.contains(link->fromNodeIndex))
        {
            link->fromNodeIndex = uuidMap[link->fromNodeIndex];
        }
        else
        {
            std::cout << "Link from node not found: " << link->fromNodeIndex << std::endl;
        }
        if (uuidMap.contains(link->toNodeIndex))
        {
            link->toNodeIndex = uuidMap[link->toNodeIndex];
        }
        else
        {
            std::cout << "Link from node not found: " << link->fromNodeIndex << std::endl;
        }
            
        // Add link to link manager
        if (linkManager->AddLink(link))
            m_pastedLinks.push_back(link);
    }
    linkManager->UpdateInputOutputLinks();
}

void ActionPaste::Undo()
{
    for (auto& node : m_pastedNodes)
    {
        m_nodeManager->RemoveNode(node);
    }
    for (auto& link : m_pastedLinks)
    {
        m_nodeManager->GetLinkManager()->RemoveLink(link);
    }
}
