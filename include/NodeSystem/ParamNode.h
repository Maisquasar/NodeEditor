#pragma once
#include <optional>
#include <unordered_map>

#include "Node.h"

//TODO: Fix the param node copy and paste + with sampler2d that add extra input

class ParamNode;

class ParamNodeManager
{
public:
    void AddParamNode(ParamNode* node, const std::string& name);
    void RemoveParamNode(ParamNode* node, const std::string& name);

    bool Exist(const std::string& name) const;
    void UpdateValue(const std::string& name, Vec4f value);
    void UpdateType(const std::string& name, Type type);

    void OnUpdateName(ParamNode* node, const std::string& prevName, const std::string& newName);
private:
    std::unordered_map<std::string, std::vector<ParamNode*>> m_paramNodes;
};


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
    void OnCreate() override;
    void OnRemove() override;

    void SetParamName(std::string name);
    void SetType(Type type);

    void SetEditable(bool editable) { m_editable = editable;}
    
    std::string GetParamName() const { return m_paramName;}
    Type GetType() const { return m_paramType;}
    void SetPreviewValue(Vec4f value) { m_previewValue = value; }
    bool HasPreviewValue() const { return m_previewValue.has_value(); }
    Vec4f GetPreviewValue() const { return m_previewValue.value_or(Vec4f(0));}

    void SetSerialize(bool serialize) { m_serialize = serialize; }

    std::string ToShader(ShaderMaker* shaderMaker, const FuncStruct& funcStruct) const override;

    bool ShouldSerialize() const { return m_serialize; }
private:
    friend class NodeTemplateHandler;
    std::string m_paramName;
    Type m_paramType;

    std::optional<Vec4f> m_previewValue;

    bool m_editable = true;

    // If true, the node will create a uniform on the shader
    bool m_serialize = true;
};
