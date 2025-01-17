#include "NodeEditor.h"

#include "NodeWindow.h"
#include "NodeSystem/ShaderMaker.h"
#include "Render/Font.h"
#include "Render/Framebuffer.h"

void NodeEditor::Initialize()
{
    Font::LoadFont();

    NodeTemplateHandler* templateHandler = NodeTemplateHandler::Create();
    templateHandler->Initialize();
}

void NodeEditor::SetShaderUpdateFunction(std::function<void(int)> func)
{
    Shader::SetUpdateValuesFunc(func);
}

NodeWindow* NodeEditor::CreateNodeWindow()
{
    auto nodeWindow = new NodeWindow();
    nodeWindow->Initialize();
    return nodeWindow;
}

void NodeEditor::DeleteNodeWindow(NodeWindow* nodeWindow)
{
    nodeWindow->Delete();
    delete nodeWindow;
    nodeWindow = nullptr;
}

void NodeEditor::SetShaderHeader(const std::string& header)
{
    ShaderMaker::SetShaderHeader(header);
}

void NodeEditor::SetShaderMainHeader(const std::string& mainHeader)
{
    ShaderMaker::SetShaderMainHeader(mainHeader);
}

void NodeEditor::SetShaderFooter(const std::string& footer)
{
    ShaderMaker::SetShaderFooter(footer);
}

void NodeEditor::SetMaterialNodeInputs(const std::vector<MaterialNodeInput>& inputs)
{
    NodeRef materialNode = NodeTemplateHandler::GetFromName("Material").node;
    materialNode->ClearInputs();
    for (auto& input : inputs)
    {
        materialNode->AddInput(input.name, input.type);
    }
}
