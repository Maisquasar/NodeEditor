#include "Render/Framebuffer.h"

#include <cassert>
#include <filesystem>
#include <fstream>

#include "NodeSystem/Node.h"
using namespace GALAXY;
#include <imgui.h>
#include <glad/glad.h>

UpdateValuesFunc Shader::m_updateValuesFunc = nullptr;

Ref<Mesh> Mesh::s_quad = nullptr;

Ref<Mesh> Mesh::CreateQuad()
{
    s_quad = std::make_shared<Mesh>();
    s_quad->Initialize(s_quadVertices);
    return s_quad;
}

bool Mesh::Initialize(const std::vector<float> vertices)
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    m_count = vertices.size() * 0.25f;

    // Use correct size calculation and pass vertex data pointer
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Configure vertex attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr); // Position (first 2 floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float))); // TexCoords (next 2 floats)
    glEnableVertexAttribArray(1);

    return true;
}


void Mesh::Draw() const
{
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_count));
    glBindVertexArray(0);
}

static std::string s_defaultVertShader = R"(#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
    TexCoords = aTexCoords;
}
)"; 
static std::string s_defaultFragShader = R"(#version 330 core
in vec2 TexCoords;
uniform float Time;
out vec4 FragColor;

void main()
{
	FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
)"; 

bool Shader::LoadDefaultShader()
{
    return Load(s_defaultVertShader.c_str(), s_defaultFragShader.c_str());
}

bool Shader::LoadDefaultVertex()
{
    return LoadVertexShader(s_defaultVertShader.c_str());
}

bool Shader::Load(const std::filesystem::path& path)
{
    m_path = path;
    
    auto vertPath = path.string() + ".vert";
    auto fragPath = path.string() + ".frag";

    if (!std::filesystem::exists(vertPath) || !std::filesystem::exists(fragPath))
        return false;

    std::ifstream vertFile(vertPath);
    std::ifstream fragFile(fragPath);
    std::string vertCode((std::istreambuf_iterator<char>(vertFile)), (std::istreambuf_iterator<char>()));
    std::string fragCode((std::istreambuf_iterator<char>(fragFile)), (std::istreambuf_iterator<char>()));

    const char* vertSource = vertCode.c_str();
    const char* fragSource = fragCode.c_str();

    return Load(vertSource, fragSource);
}

bool Shader::Load(const char* vertSource, const char* fragSource)
{
    m_program = glCreateProgram();
    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertexShader, 1, &vertSource, nullptr);
    glCompileShader(m_vertexShader);

    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(m_vertexShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }
    
    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentShader, 1, &fragSource, nullptr);
    glCompileShader(m_fragmentShader);

    
    // Check for compilation errors
    glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(m_fragmentShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }
    
    glAttachShader(m_program, m_vertexShader);
    glAttachShader(m_program, m_fragmentShader);
    
    Link();
    
    glDeleteShader(m_vertexShader);
    m_loaded = true;
    return true;
}

bool Shader::LoadVertexShader(const std::filesystem::path& vertPath)
{
    if (!std::filesystem::exists(vertPath))
        return false;
    
    std::ifstream vertFile(vertPath);
    std::string vertCode((std::istreambuf_iterator<char>(vertFile)), (std::istreambuf_iterator<char>()));
    const char* vertSource = vertCode.c_str();

    return LoadVertexShader(vertSource);
}

bool Shader::LoadVertexShader(const char* vertSource)
{
    m_program = glCreateProgram();
    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertexShader, 1, &vertSource, nullptr);
    glCompileShader(m_vertexShader);

    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(m_vertexShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }
    return true;
}

bool Shader::SetFragmentShaderContent(const std::string& string)
{
    const char* fragSource = string.c_str();
    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentShader, 1, &fragSource, nullptr);
    glCompileShader(m_fragmentShader);

    int success;
    char infoLog[512];
    // Check for compilation errors
    glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(m_fragmentShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        // TODO : Remove
        ImGui::SetClipboardText(string.c_str());
        return false;
    }
    m_fragSource = string.c_str();
    
    glAttachShader(m_program, m_fragmentShader);

    return true;
}

bool Shader::Link()
{
    glLinkProgram(m_program);
    int success;
    char infoLog[512];
    // Check for linking errors
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        m_loaded = false;
        return false;
    }
    ComputeUniforms();

    
    m_loaded = true;
    return true;
}

void Shader::ComputeUniforms()
{
    int numUniforms = 0;
    glGetProgramiv(m_program, GL_ACTIVE_UNIFORMS, &numUniforms);

    int binding = 0;
    for (GLint i = 0; i < numUniforms; ++i) {
        constexpr GLsizei bufSize = 256; // Maximum name length
        GLchar name[bufSize];
        GLsizei length;
        GLint size;
        GLenum type; 
        glGetActiveUniform(m_program, i, bufSize, &length, &size, &type, name);

        if (type != GL_SAMPLER_2D)
            continue;
			
        GLint location = glGetUniformLocation(m_program, name); // Get the location of the uniform
			
        m_uniform[location] = ++binding;
    }
}

void Shader::Use() const
{
    if (!m_loaded)
        return;
    glUseProgram(m_program);
}

bool Shader::RecompileFragmentShader()
{
    m_loaded = false;
    std::ifstream fragFile(m_path.string() + ".frag");
    std::string fragCode((std::istreambuf_iterator<char>(fragFile)), (std::istreambuf_iterator<char>()));
    const char* fragSource = fragCode.c_str();
    return RecompileFragmentShader(fragSource);
}

bool Shader::RecompileFragmentShader(const char* content)
{
    // Retrieve the existing fragment shader
    GLint attachedShaders = 0;
    GLuint shaders[2]; // Typically, a program has a vertex and fragment shader
    glGetAttachedShaders(m_program, 2, &attachedShaders, shaders);

    GLuint fragmentShader = 0;
    for (int i = 0; i < attachedShaders; i++)
    {
        GLint shaderType;
        glGetShaderiv(shaders[i], GL_SHADER_TYPE, &shaderType);
        if (shaderType == GL_FRAGMENT_SHADER)
        {
            fragmentShader = shaders[i];
            break;
        }
    }

    if (fragmentShader == 0)
    {
        std::cerr << "No fragment shader found in the program." << std::endl;
    }
    else
    {
        // Delete the old fragment shader
        glDetachShader(m_program, fragmentShader);
        glDeleteShader(fragmentShader);
    }
    // Create and compile the new fragment shader
    GLuint newFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(newFragmentShader, 1, &content, nullptr);
    glCompileShader(newFragmentShader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(newFragmentShader, GL_COMPILE_STATUS, &success);
    m_fragSource = content;
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(newFragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader Compilation Error:\n" << infoLog << std::endl;
        glDeleteShader(newFragmentShader);
        ImGui::SetClipboardText(content);
        return false;
    }

    // Attach the new shader and relink the program
    glAttachShader(m_program, newFragmentShader);
    return Link();
}

void Shader::UpdateValues()
{
    m_updateValuesFunc(m_program);
}

void Shader::SendValue(const char* name, Vec4f value, Type type)
{
    GLint location = glGetUniformLocation(m_program, name);
    if (location == -1)
        return;
    switch (type)
    {
    case Type::None:
        assert(false);
        break;
    case Type::Float:
        glUniform1f(location, value.x);
        break;
    case Type::Int:
        glUniform1i(location, value.x);
        break;
    case Type::Bool:
        glUniform1i(location, value.x);
        break;
    case Type::Vector2:
        glUniform2f(location, value.x, value.y);
        break;
    case Type::Vector3:
        glUniform3f(location, value.x, value.y, value.z);
        break;
    case Type::Vector4:
        glUniform4f(location, value.x, value.y, value.z, value.w);
        break;
    case Type::Sampler2D:
        {
            int binding = m_uniform[location];
            glActiveTexture(GL_TEXTURE0 + binding);
            glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(value.x));
            glUniform1i(location, binding);
            break;
        }
    }
}

Framebuffer::Framebuffer(){}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &m_frameBuffer);
    glDeleteRenderbuffers(1, &m_renderBuffer);
    glDeleteTextures(1, &m_texture);
}

bool Framebuffer::Initialize()
{
    bool result = true;
    glGenFramebuffers(1, &m_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_size.x, m_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;
    
    glGenRenderbuffers(1, &m_renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_size.x, m_size.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffer); 

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return true;
}

void Framebuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
}

void Framebuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Framebuffer::Update()
{
    if (m_newSize.has_value())
    {
        glViewport(0, 0, static_cast<GLsizei>(m_newSize.value().x), static_cast<GLsizei>(m_newSize.value().y));
        Resize(m_newSize.value());
        m_newSize = std::nullopt;
    }
}

void Framebuffer::Resize(const Vec2f& size)
{
    if (size.x == m_size.x && size.y == m_size.y || size.x * size.y == 0)
        return;
    m_size = size;
    
    Bind();
    glBindTexture(GL_TEXTURE_2D, m_texture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), 0, GL_RGBA, GL_FLOAT, nullptr);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));

    glBindTexture(GL_TEXTURE_2D, 0);
    Unbind();
}
