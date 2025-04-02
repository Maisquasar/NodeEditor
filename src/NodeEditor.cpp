#include "NodeEditor.h"

#include "NodeWindow.h"
#include "NodeSystem/ParamNode.h"
#include "NodeSystem/ShaderMaker.h"
#include "Render/Font.h"
#include "Render/Framebuffer.h"
#include "Render/RenderDocAPI.h"

void NodeEditor::Initialize()
{
    Font::LoadFont();

    RenderDocAPI::Initialize();
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

void NodeEditor::SetEditCustomNodeOutputPath(const std::filesystem::path& path)
{
    NodeTemplateHandler::SetTempPath(path);
}

void NodeEditor::AddInVariableNode(const std::string& name, Type type, std::string variableName, const std::vector<std::string>& searchStrings)
{
    if (variableName.empty())
        variableName = name;
    Ref<ParamNode> node = std::make_shared<ParamNode>(name);
    node->SetTopColor(NodeTemplateHandler::GetNodeColor(NodeColorType::ParamNodeColor));
    node->SetType(type);
    node->SetParamName(variableName);
    node->SetEditable(false);
    node->SetSerialize(false);
        
    NodeMethodInfo info{node};
    info.searchStrings = searchStrings;
    NodeTemplateHandler::GetInstance()->AddTemplateNode(info);
}

void NodeEditor::AddUniformNode(const std::string& name, Type type, std::string variableName)
{
    if (variableName.empty())
        variableName = name;
    Ref<ParamNode> node = std::make_shared<ParamNode>(name);
    node->SetTopColor(NodeTemplateHandler::GetNodeColor(NodeColorType::ParamNodeColor));
    node->SetType(type);
    node->SetParamName(variableName);
    node->SetEditable(false);
        
    NodeMethodInfo info{node};
    NodeTemplateHandler::GetInstance()->AddTemplateNode(info);
}

void NodeEditor::SetTextureSelectorFunction(const TextureSelectorFunc& func)
{
    TextureSelectorFunction = func;
}

void NodeEditor::SetLoadTextureFunction(const LoadTextureFunc& func)
{
    LoadTextureFunction = func;
}

bool NodeEditor::ShowTextureSelector(const char* str, int* valueInt, std::filesystem::path* outputPath)
{
    return TextureSelectorFunction(str, valueInt, outputPath);
}
