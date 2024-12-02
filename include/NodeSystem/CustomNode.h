#pragma once
#include "Node.h"
#include "Actions/ActionChangeType.h"

class CustomNode : public Node
{
public:
    CustomNode() : Node() {}
    CustomNode(const std::string& name) : Node(name) {}
    ~CustomNode() override = default;

    virtual Node* Clone() const override;

    void Update() override;
    std::filesystem::path GetTempPath() const;

    void ShowInInspector() override;

    void Serialize(CppSer::Serializer& serializer) const override;
    void Deserialize(CppSer::Parser& parser) override;
    
    std::string GetContent() const { return m_content; }

    std::string GetFunctionName() const;

    std::vector<std::string> GetFormatStrings() const override;

    std::string ToShader(ShaderMaker* shaderMaker, const FuncStruct& funcStruct) const override;

    void ClearInputs();
    void ClearOutputs();
private:
    std::string m_content;
    std::filesystem::file_time_type m_lastWriteTime;
};

using CustomNodeRef = Ref<CustomNode>;
using CustomNodeWeak = Weak<CustomNode>;