#pragma once
#include <filesystem>
#include <map>

#include "Type.h"

class Texture
{
public:
    Texture(std::filesystem::path path);
    ~Texture();

    unsigned int GetID() const { return m_id; }
private:
    unsigned int m_id;
};

class ResourceManager
{
public:
    static void Create();
    static void Destroy();
    
    static void LoadTexture(std::filesystem::path path);

    static Weak<Texture> GetOrLoadTexture(std::filesystem::path path);

    static Weak<Texture> GetTextureWithID(unsigned int id);
    
private:
    static ResourceManager* s_instance;
    
    std::map<std::filesystem::path, Ref<Texture>>  m_textures;
    
};
