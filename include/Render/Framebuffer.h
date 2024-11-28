#pragma once
#include <cstdint>
#include <Maths.h>
#include <vector>
#include <filesystem>

template <typename T>
using Ref = std::shared_ptr<T>;

static std::vector s_quadVertices = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

class Mesh
{
public:
    Mesh() = default;
    ~Mesh() = default;

    static Ref<Mesh> CreateQuad();

    bool Initialize(const std::vector<float> vertices);

    void Draw() const;
private:
    uint32_t m_vao = -1;
    uint32_t m_vbo = -1;

    uint32_t m_count;
};

class Shader
{
public:
    Shader() = default;
    ~Shader() = default;

    bool Load(const std::filesystem::path& path);

    void Use() const;

private:
    uint32_t m_program = -1;
    uint32_t m_vertexShader = -1;
    uint32_t m_fragmentShader = -1;
};

class Framebuffer
{
public:
    Framebuffer();
    ~Framebuffer();

    bool Initialize();

    void Bind() const;
    void Unbind() const;

    void Resize(const Vec2f& size);
    
private:
    Vec2f m_size = { 1280, 720 };

    uint32_t m_renderBuffer = -1;
    uint32_t m_frameBuffer = -1;
    uint32_t m_index = -1;
    uint32_t m_texture = -1;
    
};
