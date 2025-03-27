#pragma once
#include <cstdint>
#include <galaxymath/Maths.h>
#include <vector>
#include <filesystem>
#include <functional>
#include <optional>

enum class Type;
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

    static Ref<Mesh> GetQuad() { if (!s_quad) return CreateQuad(); return s_quad; }

    bool Initialize(const std::vector<float> vertices);

    void Draw() const;

    uint32_t GetVAO() const { return m_vao; }
private:
    static Ref<Mesh> s_quad;
    
    uint32_t m_vao = -1;
    uint32_t m_vbo = -1;

    uint32_t m_count;
};

using UpdateValuesFunc = std::function<void(int)>;
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

    std::filesystem::path GetPath() const { return m_path; }
    
    void ComputeUniforms();
    using UniformMap = std::unordered_map<int /* location */, int /* bind */>;
    UniformMap GetUniforms() const { return m_uniform; }

    void Use() const;
    bool RecompileFragmentShader();
    bool RecompileFragmentShader(const char* content);
    void UpdateValues();

    void SendValue(const char* name, Vec4f value, Type type);
    
    //TODO: Fix update values not called every frames
    static void SetUpdateValuesFunc(const UpdateValuesFunc& func) { m_updateValuesFunc = func; }

    bool IsLoaded() const { return m_loaded; }
    bool Failed() const { return m_failed; }
    std::string GetError() const { return m_error; }

    std::string GetFragmentSource() const { return m_fragSource; }

private:
    std::filesystem::path m_path;
    uint32_t m_program = -1;
    uint32_t m_vertexShader = -1;
    uint32_t m_fragmentShader = -1;

    static UpdateValuesFunc m_updateValuesFunc;

    bool m_loaded = false;
    bool m_failed = false;
    std::string m_error;

    std::string m_fragSource;

    UniformMap m_uniform;
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
