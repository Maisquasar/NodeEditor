#include "ResourceManager.h"

#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glad/glad.h>

Texture::Texture(std::filesystem::path path)
{
    //Load
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.generic_string().c_str(), &width, &height, &nrChannels, 4);
    
    if (data == nullptr)
    {
        std::cout << "Failed to load texture" << std::endl;
        return;
    }
    
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(data);
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_id);
}

ResourceManager* ResourceManager::s_instance = nullptr;

void ResourceManager::Create()
{
    s_instance = new ResourceManager();
}

void ResourceManager::Destroy()
{
    delete s_instance;
    s_instance = nullptr;
}

void ResourceManager::LoadTexture(std::filesystem::path path)
{
    s_instance->m_textures[path] = std::make_shared<Texture>(path);
}

Weak<Texture> ResourceManager::GetOrLoadTexture(std::filesystem::path path)
{
    if (s_instance->m_textures.find(path) == s_instance->m_textures.end())
        LoadTexture(path);
    return s_instance->m_textures[path];
}

Weak<Texture> ResourceManager::GetTextureWithID(unsigned int id)
{
    for (auto& texture : s_instance->m_textures)
    {
        if (texture.second->GetID() == id)
            return texture.second;
    }
    return {};
}