#pragma once
#include <unordered_map>
#include <optional>

#include "NodeSystem/Node.h"

class RerouteNodeNamed;
struct RerouteNodeNamedData
{
    std::string name;
    std::vector<RerouteNodeNamed*> node = {};
    int refCount = 0;
};

using RerouteNodeNamedMap = std::unordered_map<std::string, RerouteNodeNamedData>;
class RerouteNodeNamedManager
{
public:
    RerouteNodeNamedManager() = default;
    ~RerouteNodeNamedManager();

    RerouteNodeNamedData* GetNode(const std::string& name);

    void UpdateKey(std::string oldName, const std::string& newName);
    void UpdateType(const std::string& name, Type type);
    void UpdateColor(const std::string& name, const Vec3f& color);
    
    void AddNode(const std::string& name);
    void RemoveNode(const std::string& name);

    void AddRef(const std::string& name, RerouteNodeNamed* node);
    void RemoveRef(const std::string& name, RerouteNodeNamed* node);

    RerouteNodeNamed* GetDefinitionNode(const std::string& name);

    const RerouteNodeNamedData& GetNodeData(const std::string& name) const;

    bool HasDefinition(const std::string& name);
    bool HasNode(const std::string& name);
    void Clean();

private:
    RerouteNodeNamedMap m_rerouteNamedNodes;
};

class RerouteNodeNamed : public Node, public std::enable_shared_from_this<RerouteNodeNamed>
{
public:
    explicit RerouteNodeNamed(const std::string& name);
    ~RerouteNodeNamed() override;

    std::vector<std::string> GetFormatStrings() const override;
    
    void ShowInInspector() override;

    std::string ToShader(ShaderMaker* shaderMaker, const FuncStruct& funcStruct) const override;
    
    void SetType(Type type);
    void SetColor(const Vec3f& color);

    void InternalSerialize(CppSer::Serializer& serializer) const override;
    void InternalDeserialize(CppSer::Parser& parser) override;
    
    Node* Clone() const override;

    void InitializePreview() override;

    Ref<RerouteNodeNamed> GetDefinitionNode() const;
    
    bool IsDefinition() const { return m_definition; }
    void SetRerouteName(const std::string& string);
    std::string GetRerouteName() const { return m_name; }

protected:
    void OnCreate() override;
    void OnRemove() override;

private:
    std::string m_name = "Name";
    Type m_type = Type::Float;

    std::optional<Vec3f> m_color;

    bool m_templateNode = true; // This is the default template node

    bool m_definition = false;
};
