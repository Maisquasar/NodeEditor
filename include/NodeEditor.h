#pragma once
#include <functional>
#include <string>
#include <filesystem>
#include <utility>

#include "NodeSystem/Node.h"
#include "Render/Framebuffer.h"

class NodeWindow;

struct MaterialNodeInput
{
    std::string name;
    Type type;
};

/*TODO :
 * Add Output result of shader compilation, and error log
 * Add a way to not allow to make a infinite loop between nodes so it not crash
*/

namespace NodeEditor
{
    inline extern std::string TexCoordsVariableName = "TexCoords";
    /**
     * @brief Initializes the NodeEditor system.
     * This function must be called before using any other functions in the NodeEditor namespace.
     */
    void Initialize();
    
    /**
     * @brief Sets the function to be called when a shader update is triggered.
     * @param func A function that takes an integer argument that represents the program ID of the shader.
     */
    void SetShaderUpdateFunction(UpdateValuesFunc func);

    /**
     * @brief Creates a new NodeWindow instance.
     * @return A pointer to the newly created NodeWindow.
     * The caller is responsible for deleting the NodeWindow using DeleteNodeWindow.
     */
    NodeWindow* CreateNodeWindow();
    /**
     * @brief Deletes a NodeWindow instance and frees its resources.
     * @param nodeWindow A pointer to the NodeWindow to be deleted.
     */
    void DeleteNodeWindow(NodeWindow* nodeWindow);

    /**
     * @brief Sets the header for the generated shader code.
     * @param header A string containing the header code.
     * This is typically used to define global variables.
     */
    void SetShaderHeader(const std::string& header);
    /**
     * @brief Sets the header for the shader's main function.
     * @param mainHeader A string containing the main function's header code.
     * Use this to define custom setups or include necessary declarations.
     */
    void SetShaderMainHeader(const std::string& mainHeader);
    /**
     * @brief Sets the footer for the generated shader code.
     * @param footer A string containing the footer code and %s that represents each variable of the material node.
     * This is often used for additional code at the end of the shader.
     */
    void SetShaderFooter(const std::string& footer);
    /**
     * @brief Sets the material node inputs for the shader.
     * @param inputs A vector of MaterialNodeInput objects defining the shader's input parameters.
     * Use this to configure the inputs for material-related nodes in the editor.
     */
    void SetMaterialNodeInputs(const std::vector<MaterialNodeInput>& inputs);
    
    /**
     * @brief Sets the output path for saving custom node edits so that the ide can access them.
     * @param path A filesystem path specifying where custom node output should be saved.
     * @note the files will not be deleted automatically, so make sure to delete them manually if you need to.
     */
    void SetEditCustomNodeOutputPath(const std::filesystem::path& path);
    
    /**
     * @brief Adds an input variable node to the editor that come from the vertex shader.
     * @param name The name of the node.
     * @param type The type of the variable (e.g., float, int, vec3).
     * @param variableName (Optional) The variable name to be used in the shader and in the output.
     * If not provided, the default name will be the same as the node name.
     * @param searchStrings (Optional) A vector of strings that will be used to search for the variable in the shader. 
     * @note The in variable should be set in the header of the shader,
     * it will not be created automatically not like the uniform.
     */
    void AddInVariableNode(const std::string& name, Type type, std::string variableName = "", const std::vector<std::string>& searchStrings = {});
    /**
     * @brief Adds a uniform variable node to the editor.
     * @param name The name of the node.
     * @param type The type of the variable (e.g., float, int, vec3).
     * @param variableName (Optional) The variable name to be used in the shader.
     * If not provided, the default name will be the same as the node name.
     * @note The uniform variable will be created automatically in the shader, no need to set it in the header of the shader.
     * However, you need to send the value to the shader with the ShaderUpdateFunction
     */
    void AddUniformNode(const std::string& name, Type type, std::string variableName = "");

    inline void SetTexCoordsVariableName(std::string name) { TexCoordsVariableName = std::move(name); }

    using TextureSelectorFunc = std::function<bool(const char*, int*, std::filesystem::path*)>;
    using LoadTextureFunc = std::function<bool(const std::filesystem::path&, Node*)>;
    void SetTextureSelectorFunction(const TextureSelectorFunc& func);
    void SetLoadTextureFunction(const LoadTextureFunc& func);
    
    bool ShowTextureSelector(const char* str, int* valueInt, std::filesystem::path* outputPath);

    inline extern TextureSelectorFunc TextureSelectorFunction = nullptr;
    inline extern LoadTextureFunc LoadTextureFunction = nullptr;
};
