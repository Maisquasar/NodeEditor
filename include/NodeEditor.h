#pragma once
#include <functional>
#include <string>

#include "NodeSystem/Node.h"

class NodeWindow;

struct MaterialNodeInput
{
    std::string name;
    Type type;
};

namespace NodeEditor
{
    void Initialize();

    void SetShaderUpdateFunction(std::function<void(int)> func);

    NodeWindow* CreateNodeWindow();
    void DeleteNodeWindow(NodeWindow* nodeWindow);

    void SetShaderHeader(const std::string& header);
    void SetShaderMainHeader(const std::string& mainHeader);
    void SetShaderFooter(const std::string& footer);
    void SetMaterialNodeInputs(const std::vector<MaterialNodeInput>& inputs);
    
};
