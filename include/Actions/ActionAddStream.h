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
private:
    CustomNode* m_node;
    std::string m_name;
    StreamRef m_stream;

    std::vector<Link> m_prevLinks = {};
};

