#pragma once

#include <string>
#include "stb_image.h"

static std::string read_file(const char* filepath)
{
    FILE* file = fopen(filepath, "rt");
    fseek(file, 0, SEEK_END);
    unsigned long length = ftell(file);
    unsigned long terminated_length = length + 1;
    char* data = new char[terminated_length];
    memset(data, 0, terminated_length);
    fseek(file, 0, SEEK_SET);
    fread(data, 1, length, file);
    fclose(file);

    std::string result(data);
    delete[] data;
    return result;
}

static GLuint TextureFromFile(const char* path, const std::string& directory, bool gamma = false)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum in_format = GL_RED, out_format = GL_RED;
        if (nrChannels == 3)
        {
            in_format = gamma ? GL_SRGB : GL_RGB;
            out_format = GL_RGB;
        }
        else if (nrChannels == 4)
        {
            in_format = gamma ? GL_SRGB_ALPHA : GL_RGBA;
            out_format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, in_format, width, height, 0, out_format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, out_format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, out_format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cout << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}
