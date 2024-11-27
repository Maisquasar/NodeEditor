#pragma once
#include "Node.h"

class ParamNode : public Node
{
public:
    ParamNode() : Node() {}
    ParamNode(std::string name) : Node(name) {}
    ~ParamNode() override = default;

    void ShowInInspector() override;

    std::vector<std::string> GetFormatStrings() const override;

    void InternalSerialize(CppSer::Serializer& serializer) const override;
    void InternalDeserialize(CppSer::Parser& parser) override;

    Node* Clone() const override;
    
    void SetType(Type type);

private:
    std::string m_paramName = "None";
    Type m_paramType;
};
