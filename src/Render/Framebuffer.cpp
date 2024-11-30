#include "Render/Framebuffer.h"

#include <filesystem>
#include <fstream>
#include <glad/glad.h>

#include "Application.h"

Ref<Mesh> Mesh::CreateQuad()
{
    Ref<Mesh> mesh = std::make_shared<Mesh>();
    mesh->Initialize(s_quadVertices);
    return mesh;
}

bool Mesh::Initialize(const std::vector<float> vertices)
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    m_count = vertices.size() / 4;

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
        return false;
    }
    
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
        return false;
    }
    return true;
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
    glShaderSource(newFragmentShader, 1, &fragSource, nullptr);
    glCompileShader(newFragmentShader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(newFragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(newFragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader Compilation Error:\n" << infoLog << std::endl;
        glDeleteShader(newFragmentShader);
        return false;
    }

    // Attach the new shader and relink the program
    glAttachShader(m_program, newFragmentShader);
    glLinkProgram(m_program);

    // Check for linking errors
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
        std::cerr << "Program Linking Error:\n" << infoLog << std::endl;
        return false;
    }
    m_loaded = true;
    return true;
}

void Shader::UpdateValues()
{
    // Set time value
    GLint timeLocation = glGetUniformLocation(m_program, "Time");
    if (timeLocation != -1) {
        float time = Application::GetInstance()->GetTime();
        glUniform1f(timeLocation, time);
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
