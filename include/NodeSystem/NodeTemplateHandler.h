#pragma once
#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "NodeSystem/Node.h"

class Node;

using OutputFormatString = std::vector<std::string>;

struct NodeMethodInfo
{
    NodeMethodInfo() = default;
    NodeMethodInfo(Node* ptr) : node(ptr) {}
    NodeMethodInfo(NodeRef ref) : node(ref) {}
    NodeMethodInfo(NodeRef ref, std::vector<std::string> _formatStrings);
    
    NodeRef node;
    std::vector<OutputFormatString> outputFormatStrings;
    std::vector<std::string> searchStrings;
    int tabIndex = -1;
};

using TemplateList = std::vector<NodeMethodInfo>;

struct StreamNameType
{
    StreamNameType() = default;
    StreamNameType(std::string _name, Type _type) : name(std::move(_name)), type(_type) {}

    std::string name;
    Type type;
};

enum class NodeColorType
{
    EndColor,
    FunctionColor,
    MakeColor,
    BreakColor,
    CustomNodeColor,
    ParamNodeColor,
    OtherNodeColor,
};

class NodeTemplateHandler
{
public:
    NodeTemplateHandler() = default;

    static NodeTemplateHandler* Create() { return (s_instance = std::make_unique<NodeTemplateHandler>()).get(); }
    static NodeTemplateHandler* GetInstance() { return s_instance.get(); }
    
    static void SetTempPath(const std::filesystem::path& path) { s_instance->m_tempPath = path; }
    static std::filesystem::path GetTempPath() {return s_instance->m_tempPath; }

    static uint32_t GetNodeColor(NodeColorType type);
    
    void RunUnitTests();
    bool RunUnitTest(const NodeMethodInfo& info);
    
    void Initialize();

    void ComputeNodesSize();

    static std::vector<std::string> GetTemplateFormatStrings(TemplateID templateID, int possibilityIndex);

    void AddTemplateNode(const NodeMethodInfo& info);
    static TemplateID TemplateIDFromString(const std::string& name);

    static std::shared_ptr<Node> CreateFromTemplate(TemplateID templateID);

    static std::shared_ptr<Node> CreateFromTemplateName(const std::string& name);

    void RemoveTemplateNode(const std::string& name);

    TemplateList& GetTemplates() { return m_templateNodes; }
    static NodeMethodInfo& GetFromName(const std::string& name);

    static NodeRef GetNodeFromName(const std::string& name);

    static void UpdateKey(const std::string& oldName, const std::string& newName);
    static void UpdateType(const std::string& name, Type type);
    static void UpdateColor(const std::string& name, uint32_t color);

    static bool DoesNameExist(const std::string& name);
private:
    void CreateTemplateNode(const std::string& name, const uint32_t& color, const std::vector<StreamNameType>& inputs,
                            const std::vector<StreamNameType>& outputs, const std::vector<std::string>& format,
                            const std::vector<std::string>& searchStrings = {});

    void CreateTemplateNode(const std::string& name, const uint32_t& color, const std::vector<StreamNameType>& inputs,
                            const std::vector<StreamNameType>& outputs, const std::string& format,
                            const std::vector<std::string>& searchStrings = {});

private:
    static std::unique_ptr<NodeTemplateHandler> s_instance;

    TemplateList m_templateNodes;

    bool m_computed = false;
    
    std::filesystem::path m_tempPath;
};
