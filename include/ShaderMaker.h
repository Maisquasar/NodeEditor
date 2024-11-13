#pragma once
#include <deque>

#include "NodeManager.h"
#include "NodeTemplateHandler.h"

class ShaderMaker
{
public:
    void RecurrenceWork(NodeManager* manager, const NodeRef& endNode, TemplateList& templateList, std::string& content,
                       LinkManager* linkManager);
    void CreateFragmentShader(NodeManager* manager);

    std::deque<std::string> m_variablesNames;
};
