#pragma once
#include "Action.h"
#include "NodeSystem/CustomNode.h"

class ActionAddStream : public Action
{
public:
    ActionAddStream(CustomNode* node, const std::string& name, Type type, StreamType streamType);
    void Do() override;
    void Undo() override;
    std::string ToString() override { return "Add Stream"; }
    bool ShouldUpdateShader() const override { return true; }
    std::unordered_set<UUID> NodeToUpdate() const override;
private:
    CustomNode* m_node;
    std::string m_name;
    Type m_type;
    StreamType m_streamType;
};

class ActionRemoveStream : public Action
{
public:
    ActionRemoveStream(CustomNode* node, StreamRef stream);
    void Do() override;
    void Undo() override;
    std::string ToString() override { return "Remove Stream"; }
    bool ShouldUpdateShader() const override { return true; }
    std::unordered_set<UUID> NodeToUpdate() const override;
private:
    CustomNode* m_node;
    std::string m_name;
    StreamRef m_stream;

    std::vector<Link> m_prevLinks = {};
};

class ActionChangeStreamType : public Action
{
public:
    ActionChangeStreamType(Node* node, int index);
    void Do() override;
    void Undo() override;
    
    std::string ToString() override { return "Change Stream Type"; }
    bool ShouldUpdateShader() const override { return true; }
    std::unordered_set<UUID> NodeToUpdate() const override;
private:
    std::vector<Link> m_prevLinks = {};
    Node* m_node;
    int m_index;
    int m_prevIndex;
};