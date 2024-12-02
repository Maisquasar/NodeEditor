#include "NodeSystem/CustomNode.h"

#include <CppSerializer.h>

#include "imgui_stdlib.h"
#include "NodeWindow.h"
#include "Actions/ActionChangeInput.h"
#include "Actions/ActionChangeType.h"
#include "NodeSystem/ShaderMaker.h"

Node* CustomNode::Clone() const
{
    auto node = new CustomNode();
    Internal_Clone(node);
    return node;
}

void CustomNode::Update()
{
    auto path = std::filesystem::current_path().generic_string() + GetTempPath().generic_string();
    if (!std::filesystem::exists(path))
        return;
    if (std::filesystem::last_write_time(path) <= m_lastWriteTime)
        return;

    m_lastWriteTime = std::filesystem::last_write_time(path);
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
    m_content = content;
}

std::filesystem::path CustomNode::GetTempPath() const
{
    return TEMP_FOLDER + (std::to_string(p_uuid) + ".frag");
}

void CustomNode::ShowInInspector()
{
    Node::ShowInInspector();

    ImGui::SeparatorText("Inputs");
    ImGui::PushID("Inputs");

    if (ImGui::Button("+"))
    {
        AddInput("Input " + std::to_string(p_inputs.size()), Type::Float);
    }
    if (!p_inputs.empty())
    {
        ImGui::SameLine();
        if (ImGui::Button("-"))
        {
            RemoveInput(p_inputs.size() - 1);
        }
    }
    for (int i = 0; i < p_inputs.size(); i++)
    {
        ImGui::PushID(i);
        if (ImGui::TreeNode("##", "Input %d", i))
        {
            std::string name = p_inputs[i]->name;
            if (ImGui::InputText("Input Name", &name))
            {
                Ref<ActionChangeInput> changeInput = std::make_shared<ActionChangeInput>(&p_inputs[i]->name, p_inputs[i]->name, name);
                ActionManager::DoAction(changeInput);
            }
            int type = static_cast<int>(p_inputs[i]->type) - 1;
            if (ImGui::Combo("Type", &type, SerializeTypeEnum()))
            {
                Ref<ActionChangeType> changeType = std::make_shared<ActionChangeType>(this, p_inputs[i], static_cast<Type>(type + 1), p_inputs[i]->type);
                ActionManager::AddAction(changeType);
                p_nodeManager->GetLinkManager()->RemoveLink(p_inputs[i]);
                p_inputs[i]->type = static_cast<Type>(type + 1);
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::PopID();
    ImGui::SeparatorText("Outputs");
    ImGui::PushID("Outputs");

    if (ImGui::Button("+"))
    {
        AddOutput("Output" + std::to_string(p_outputs.size()), Type::Float);
    }
    if (!p_outputs.empty())
    {
        ImGui::SameLine();
        if (ImGui::Button("-"))
        {
            RemoveOutput(p_outputs.size() - 1);
        }
    }
    for (int i = 0; i < p_outputs.size(); i++)
    {
        ImGui::PushID(i);
        if (ImGui::TreeNode("##", "Output %d", i))
        {
            std::string name = p_outputs[i]->name;
            if (ImGui::InputText("Output Name", &name))
            {
                Ref<ActionChangeInput> changeInput = std::make_shared<ActionChangeInput>(&p_outputs[i]->name, p_outputs[i]->name, name);
                ActionManager::DoAction(changeInput);
            }
            int type = static_cast<int>(p_outputs[i]->type) - 1;
            if (ImGui::Combo("Type", &type, SerializeTypeEnum()))
            {
                Ref<ActionChangeType> changeType = std::make_shared<ActionChangeType>(this, p_outputs[i], static_cast<Type>(type + 1), p_outputs[i]->type);
                ActionManager::AddAction(changeType);
                p_nodeManager->GetLinkManager()->RemoveLinks(p_outputs[i]);
                p_outputs[i]->type = static_cast<Type>(type + 1);
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    ImGui::PopID();
    ImGui::Separator();

    if (ImGui::Button("Edit on vscode"))
    {
        std::filesystem::path tempPath = std::filesystem::current_path().generic_string() + GetTempPath().generic_string();
        std::cout << tempPath << std::filesystem::current_path();
        std::ofstream file(tempPath);
        if (file.is_open())
        {
            file << m_content;
            file.close();
            std::string command = "code \"" + tempPath.generic_string() + "\"";
            std::system(command.c_str());
            m_lastWriteTime = std::filesystem::last_write_time(tempPath);
        }
    }
    ImGui::InputTextMultiline("Content", &m_content);
}

// Function to replace all occurrences of a substring with another substring
void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Advance to avoid infinite loop
    }
}

// Sanitize function: Replaces actual newlines and tabs with literal \n and \t
void SanitizeString(std::string& str) {
    ReplaceAll(str, "\n", "\\n"); // Replace newline with \n
    ReplaceAll(str, "\t", "\\t"); // Replace tab with \t
}

// Unsanitize function: Replaces literal \n and \t back to newlines and tabs
void UnsanitizeString(std::string& str) {
    ReplaceAll(str, "\\n", "\n"); // Replace \n with actual newline
    ReplaceAll(str, "\\t", "\t"); // Replace \t with actual tab
}


void CustomNode::Serialize(CppSer::Serializer& serializer) const
{
    serializer << CppSer::Pair::BeginMap << "Node";
    serializer << CppSer::Pair::Key << "TemplateID" << CppSer::Pair::Value << p_templateID;
    serializer << CppSer::Pair::Key << "UUID" << CppSer::Pair::Value << p_uuid;
    serializer << CppSer::Pair::Key << "Position" << CppSer::Pair::Value << p_position;

    serializer << CppSer::Pair::Key << "Input Count" << CppSer::Pair::Value << p_inputs.size();
    for (uint32_t i = 0; i < p_inputs.size(); i++)
    {
        auto& input = p_inputs[i];

        serializer << CppSer::Pair::Key << "Input Name " + std::to_string(i) << CppSer::Pair::Value << input->name;
        serializer << CppSer::Pair::Key << "Input Type " + std::to_string(i) << CppSer::Pair::Value << static_cast<int>(input->type);
        if (input->isLinked)
            continue;

        serializer << CppSer::Pair::Key << "Value " + std::to_string(i) << CppSer::Pair::Value << input->GetValue();
    }

    serializer << CppSer::Pair::Key << "Output Count" << CppSer::Pair::Value << p_outputs.size();
    for (uint32_t i = 0; i < p_outputs.size(); i++)
    {
        auto& output = p_outputs[i];
        serializer << CppSer::Pair::Key << "Output Name " + std::to_string(i) << CppSer::Pair::Value << output->name;
        serializer << CppSer::Pair::Key << "Output Type " + std::to_string(i) << CppSer::Pair::Value << static_cast<int>(output->type);
    }

    std::string content = m_content;
    SanitizeString(content);

    serializer << CppSer::Pair::Key << "Content" << CppSer::Pair::Value << content;

    InternalSerialize(serializer);
    
    serializer << CppSer::Pair::EndMap << "Node";
}

void CustomNode::Deserialize(CppSer::Parser& parser)
{
    p_uuid = parser["UUID"].As<uint64_t>();
    SetUUID(p_uuid);
    p_position = parser["Position"].As<Vec2f>();

    int inputCount = parser["Input Count"].As<int>();

    p_inputs.clear();
    for (uint32_t i = 0; i < inputCount; i++)
    {
        std::string inputName = parser["Input Name " + std::to_string(i)].As<std::string>();
        Type inputType = static_cast<Type>(parser["Input Type " + std::to_string(i)].As<int>());
        AddInput(inputName, inputType);
        auto& input = p_inputs[i];
        if (input->isLinked)
            continue;

        std::string key = "Value " + std::to_string(i);
        input->SetValue(parser[key].As<Vec4f>());
    }

    int outputCount = parser["Output Count"].As<int>();

    p_outputs.clear();
    for (uint32_t i = 0; i < outputCount; i++)
    {
        std::string outputName = parser["Output Name " + std::to_string(i)].As<std::string>();
        Type outputType = static_cast<Type>(parser["Output Type " + std::to_string(i)].As<int>());
        AddOutput(outputName, outputType);
    }

    m_content = parser["Content"].As<std::string>();
    UnsanitizeString(m_content);

    InternalDeserialize(parser);
}

std::string CustomNode::GetFunctionName() const
{
    auto name = GetName();
    ShaderMaker::CleanString(name);
    return name + "_" + std::to_string(p_uuid) + "_" + "Func";
}

std::vector<std::string> CustomNode::GetFormatStrings() const
{
    std::vector<std::string> formatStrings = {};
    std::string format = GetFunctionName() + "(";
    for (int i = 0; i < p_inputs.size(); i++)
    {
        format += "%s, ";
    }
    for (int i = 0; i < p_outputs.size(); i++)
    {
        format += "%s, ";
    }
    format.erase(format.end() - 2, format.end());
    format += ")";
    formatStrings.push_back(format);
    return formatStrings;
}

std::string CustomNode::ToShader(ShaderMaker* shaderMaker, const FuncStruct& funcStruct) const
{
    std::string content;
    auto& variableNames = shaderMaker->m_variablesNames;
    auto& allVariableNames = shaderMaker->m_allVariableNames;
    auto toFormatList = GetFormatStrings();
    for (int k = 0; k < p_outputs.size(); k++)
    {
        std::string variableName = funcStruct.outputs[k];
        std::string glslType = ShaderMaker::TypeToGLSLType(p_outputs[k]->type);

        if (allVariableNames.contains(variableName))
            continue;
        allVariableNames.insert(variableName);
        std::string thisContent = glslType + " " + variableName + ";\n";
        content += thisContent;
    }
        
    std::string toFormat = toFormatList[0];
    std::string secondHalf = toFormat;
    toFormat.clear();
    for (int j = 0; j < p_inputs.size(); j++)
    {
        InputRef input = p_inputs[j];
        size_t index = secondHalf.find_first_of("%") + 2;
        if (index == std::string::npos)
            break;
        std::string firstHalf = secondHalf.substr(0, index);
        if (index != std::string::npos)
            secondHalf = secondHalf.substr(index);
        if (!input->isLinked)
        {
            variableNames.push_back(ShaderMaker::GetValueAsString(input));
        }
        auto parentVariableName = funcStruct.inputs[j];
        if (parentVariableName.empty())
            parentVariableName = variableNames.back();
        toFormat += FormatString(firstHalf, parentVariableName.c_str());
    }
    for (int j = 0; j < p_outputs.size(); j++)
    {
        OutputRef output = p_outputs[j];
        size_t index = secondHalf.find_first_of("%") + 2;
        if (index == std::string::npos)
            break;
        std::string firstHalf = secondHalf.substr(0, index);
        if (index != std::string::npos)
            secondHalf = secondHalf.substr(index);
        auto parentVariableName = funcStruct.outputs[j];
        toFormat += FormatString(firstHalf, parentVariableName.c_str());
    }
    content += toFormat + secondHalf + ";\n";
    
    return content;
}

void CustomNode::ClearInputs()
{
    for (int i = 0; i < p_inputs.size(); i++)
    {
        RemoveInput(i--);
    }
    p_inputs.clear();
}

void CustomNode::ClearOutputs()
{
    for (int i = 0; i < p_outputs.size(); i++)
    {
        RemoveOutput(i--);
    }
    p_outputs.clear();
}
