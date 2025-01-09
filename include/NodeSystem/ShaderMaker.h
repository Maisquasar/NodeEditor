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


template<typename ... Args>
std::string FormatString( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

class ShaderMaker
{
public:
    void FormatWithType(std::string& toFormat, InputRef input, std::string firstHalf);
    
    void FillFunctionList(NodeManager* manager, NodeRef firstNode);
    void FillRecurrence(NodeManager* manager, const NodeRef& node);

    void DoWork(NodeManager* manager);
    void CreateFragmentShader(std::string& content, NodeManager* manager);
    void CreateFragmentShader(const std::filesystem::path& path, NodeManager* manager);
    void CreateFragmentShader(std::string& content, NodeManager* manager, const NodeRef& endNode);
    void CreateShaderToyShader(NodeManager* manager);
    void SerializeFunctions(NodeManager* manager, const NodeRef& node, std::string& content);
    static std::string ToGLSLVariable(Type type, const Vec4f& value);

    static std::string GetValueAsString(InputRef input);

    static void CleanString(std::string& name);
    static std::string GetOutputVariableName(NodeRef currentNode, int j);
    static std::string TypeToGLSLType(Type type);
private:
    friend class RerouteNodeNamed;
    friend class Node;
    friend class CustomNode;
    std::unordered_map<UUID, FuncStruct> m_functions;

    std::deque<std::string> m_variablesNames;
    std::set<std::string> m_allVariableNames;
    std::vector<NodeWeak> m_nodesToSerialize;
};
