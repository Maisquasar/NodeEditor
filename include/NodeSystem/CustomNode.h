#pragma once
#include "Node.h"

class CustomNode : public Node
{
public:
    CustomNode() : Node() {}
    CustomNode(const std::string& name) : Node(name) {}
    ~CustomNode() override = default;

    virtual Node* Clone() const override;

    void ShowInInspector() override;

    void Serialize(CppSer::Serializer& serializer) const override;
    void Deserialize(CppSer::Parser& parser) override;
    
    std::string GetContent() const { return m_content; }

    std::string GetFunctionName() const;

    std::vector<std::string> GetFormatStrings() const override;

    std::string ToShader(ShaderMaker* shaderMaker, const FuncStruct& funcStruct) const override;

private:
    void ClearInputs();
    void ClearOutputs();
private:
    std::string m_content;
};

using CustomNodeRef = Ref<CustomNode>;
using CustomNodeWeak = Weak<CustomNode>;