#pragma once
#include <cstdint>
#include <Maths.h>
#include <vector>
#include <filesystem>
#include <optional>

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

    uint32_t GetVAO() const { return m_vao; }
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

    bool LoadDefaultShader();
    bool LoadDefaultVertex();
    bool Load(const std::filesystem::path& path);
    bool Load(const char* vertSource, const char* fragSource);
    bool LoadVertexShader(const std::filesystem::path& vertPath);
    bool LoadVertexShader(const char* vertSource);
    bool SetFragmentShaderContent(const std::string& string);
    bool Link();

    void Use() const;
    bool RecompileFragmentShader();
    bool RecompileFragmentShader(const char* content);
    void UpdateValues() const;

    bool IsLoaded() const { return m_loaded; }

private:
    std::filesystem::path m_path;
    uint32_t m_program = -1;
    uint32_t m_vertexShader = -1;
    uint32_t m_fragmentShader = -1;

    bool m_loaded = false;
};

class Framebuffer
{
public:
    Framebuffer();
    ~Framebuffer();

    bool Initialize();

    void Bind() const;
    void Unbind() const;

    void Update();
    void Resize(const Vec2f& size);

    uint32_t GetRenderTexture() const { return m_texture; }

    void SetNewSize(const Vec2f& size) { m_newSize = size; }
private:
    Vec2f m_size = { 1280, 720 };

    uint32_t m_renderBuffer = -1;
    uint32_t m_frameBuffer = -1;
    uint32_t m_index = -1;
    uint32_t m_texture = -1;

    std::optional<Vec2f> m_newSize;
};
