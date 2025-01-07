#pragma once
#include <unordered_map>

#include "NodeSystem/Node.h"

class RerouteNodeNamed;
struct RerouteNodeNamedData
{
    std::string name;
    std::vector<RerouteNodeNamed*> node = {};
    int refCount = 0;
};

class RerouteNodeNamedManager
{
public:
    RerouteNodeNamedManager() = default;
    ~RerouteNodeNamedManager() = default;

    static RerouteNodeNamedManager* GetInstance();

    static RerouteNodeNamedData* GetNode(const std::string& name);

    static void UpdateKey(std::string oldName, const std::string& newName);
    static void UpdateType(const std::string& name, Type type);

    static void AddNode(const std::string& name);
    static void RemoveNode(const std::string& name);

    static void AddRef(const std::string& name, RerouteNodeNamed* node);
    static void RemoveRef(const std::string& name, RerouteNodeNamed* node);

    static RerouteNodeNamed* GetDefinitionNode(const std::string& name);

    static bool HasDefinition(const std::string& name);
    static bool HasNode(const std::string& name);
private:
    static std::unique_ptr<RerouteNodeNamedManager> s_instance;

    std::unordered_map<std::string, RerouteNodeNamedData> m_rerouteNamedNodes;
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

    void InternalSerialize(CppSer::Serializer& serializer) const override;
    void InternalDeserialize(CppSer::Parser& parser) override;
    
    Node* Clone() const override;

    void InitializePreview() override;

    Ref<RerouteNodeNamed> GetDefinitionNode() const;
    
    bool IsDefinition() const { return m_definition; }
    void SetRerouteName(const std::string& string);

protected:
    void OnCreate() override;
    void OnRemove() override;

private:
    std::string m_name = "Name";
    Type m_type = Type::Float;

    bool m_templateNode = true; // This is the default template node

    bool m_definition = false;
};
