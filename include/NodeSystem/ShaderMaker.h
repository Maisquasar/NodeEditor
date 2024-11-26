#pragma once
#include <deque>
#include <set>

#include "NodeSystem/NodeManager.h"
#include "NodeSystem/NodeTemplateHandler.h"

struct FuncStruct
{
    std::string debugName;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
};

class ShaderMaker
{
public:
    void FormatWithType(std::string& toFormat, InputRef input, std::string firstHalf);
    void RecurrenceWork(NodeManager* manager, const NodeRef& endNode, TemplateList& templateList, std::string& content,
                        LinkManager* linkManager, bool insert = true);
    
    void FillFunctionList(NodeManager* manager);
    void FillRecurrence(NodeManager* manager, const NodeRef& node, const NodeRef& parentNode);
    
    void CreateFragmentShader(NodeManager* manager);
    void SerializeFunctions(NodeManager* manager, const NodeRef& node, std::string& content);

    std::string GetValueAsString(InputRef input);

    static std::string TypeToGLSLType(Type type);
private:
    std::unordered_map<UUID, FuncStruct> m_functions;

    std::deque<std::string> m_variablesNames;
    std::set<std::string> m_allVariableNames;
};
