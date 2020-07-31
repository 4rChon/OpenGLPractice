#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

class Shader
{
    const GLuint ID;
public:
    Shader(const GLsizei shaderCount, const GLuint* shaderIDs);
    ~Shader();
    static GLuint createShader(const char* path, GLenum shaderType, std::vector<const char*> preprocessor = {});
    void use() const;
    void bindUniformBlock(const char* name, GLuint index) const;
    void setBool(const char* name, bool value) const;
    void setInt(const char* name, int value) const;
    void setFloat(const char* name, float value) const;
    void setVec4(const char* name, const glm::vec4& value) const;
    void setVec3(const char* name, const glm::vec3& value) const;
    void setVec2(const char* name, const glm::vec2& value) const;
    void setMat4(const char* name, const glm::mat4& value) const;
};