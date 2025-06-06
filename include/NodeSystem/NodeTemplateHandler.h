﻿#pragma once
#include <memory>
#include <string>
#include <vector>

#include "NodeSystem/Node.h"

class Node;

struct NodeMethodInfo
{
    NodeMethodInfo() = default;
    NodeMethodInfo(NodeRef ref) : node(ref) {}
    NodeMethodInfo(NodeRef ref, std::string _formatString) : node(ref), outputFormatStrings({std::move(_formatString)}) {}
    NodeMethodInfo(NodeRef ref, std::vector<std::string> _formatStrings) : node(ref), outputFormatStrings(std::move(_formatStrings)) {}

    template<typename ... Args>
    NodeMethodInfo(NodeRef ref, std::string _formatString, Args ... args) : node(ref)
    {
        outputFormatStrings.push_back(_formatString);

        for (auto& arg : {args...})
        {
            outputFormatStrings.push_back(arg);
        }
    }
    

    NodeRef node;
    std::vector<std::string> outputFormatStrings;
    std::vector<std::string> searchStrings;
};

using TemplateList = std::vector<NodeMethodInfo>;
class NodeTemplateHandler
{
public:
    NodeTemplateHandler() = default;

    static NodeTemplateHandler* Create() { return (s_instance = std::make_unique<NodeTemplateHandler>()).get(); }
    static NodeTemplateHandler* GetInstance() { return s_instance.get(); }

    void RunUnitTests();
    bool RunUnitTest(const NodeMethodInfo& info);
    
    void Initialize();

    void ComputeNodesSize();

    static std::vector<std::string> GetTemplateFormatStrings(TemplateID templateID);

    void AddTemplateNode(const NodeMethodInfo& info);
    static TemplateID TemplateIDFromString(const std::string& name);

    static std::shared_ptr<Node> CreateFromTemplate(TemplateID templateID);

    static std::shared_ptr<Node> CreateFromTemplateName(const std::string& name);

    TemplateList& GetTemplates() { return m_templateNodes; }
private:
    void CreateTemplateNode(const std::string& name, const uint32_t& color, const std::vector<std::tuple<std::string, Type>>& inputs,
                            const std::vector<std::tuple<std::string, Type>>& outputs, const std::vector<std::string>& format,
                            const std::vector<std::string>& searchStrings = {});
    
    void CreateTemplateNode(const std::string& name, const uint32_t& color, const std::vector<std::tuple<std::string, Type>>& inputs,
                            const std::vector<std::tuple<std::string, Type>>& outputs, const std::string& format,
                            const std::vector<std::string>& searchStrings = {});

private:
    static std::unique_ptr<NodeTemplateHandler> s_instance;

    TemplateList m_templateNodes;

    bool m_computed = false;
};
