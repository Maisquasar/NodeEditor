#pragma once
#include <deque>
#include <set>

#include "NodeManager.h"
#include "NodeTemplateHandler.h"

class ShaderMaker
{
public:
    void FormatWithType(std::string& toFormat, InputRef input, std::string firstHalf);
    void RecurrenceWork(NodeManager* manager, const NodeRef& endNode, TemplateList& templateList, std::string& content,
                        LinkManager* linkManager, bool insert = true);
    void CreateFragmentShader(NodeManager* manager);

    std::string GetValueAsString(InputRef input);

    static std::string TypeToGLSLType(Type type);
private:

    std::deque<std::string> m_variablesNames;
    std::set<std::string> m_allVariableNames;
};
