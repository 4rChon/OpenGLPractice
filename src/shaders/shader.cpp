#include <iostream>
#include <string>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "../utils/fileutils.h"

Shader::Shader(const GLsizei shaderCount, const GLuint* shaderIDs)
    : ID(glCreateProgram())
{
    for (GLsizei i = 0; i < shaderCount; i++)
    {
        glAttachShader(ID, shaderIDs[i]);
        glDeleteShader(shaderIDs[i]);
    }

    glLinkProgram(ID);

#if _DEBUG
    GLint success;
    char infoLog[512];
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::LINKING\n" << infoLog << std::endl;
    }
#endif
}

Shader::~Shader()
{
    glDeleteProgram(ID);
}

GLuint Shader::createShader(const char* path, GLenum shaderType, std::vector<const char*> preprocessor)
{
    GLuint shaderID = glCreateShader(shaderType);
    std::string shaderCode = read_file(path);
    preprocessor.push_back(shaderCode.c_str());
    glShaderSource(shaderID, preprocessor.size(), &preprocessor[0], NULL);
    glCompileShader(shaderID);

#if _DEBUG
    GLint success;
    char infoLog[512];
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILE: " << path << "\n" << infoLog << std::endl;
        return false;
    }
#endif
    return shaderID;
}

void Shader::use() const
{
    glUseProgram(ID);
}

void Shader::bindUniformBlock(const char* name, GLuint index) const
{
    GLuint uniformBlockIndex = glGetUniformBlockIndex(ID, name);
    glUniformBlockBinding(ID, uniformBlockIndex, index);
}

void Shader::setBool(const char* name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name), (int)value);
}
void Shader::setInt(const char* name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name), value);
}
void Shader::setFloat(const char* name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name), value);
}

void Shader::setVec4(const char* name, const glm::vec4& value) const
{
    glUniform4f(glGetUniformLocation(ID, name), value.x, value.y, value.z, value.w);
}

void Shader::setVec3(const char* name, const glm::vec3& value) const
{
    glUniform3f(glGetUniformLocation(ID, name), value.x, value.y, value.z);
}
void Shader::setVec2(const char* name, const glm::vec2& value) const
{
    glUniform2f(glGetUniformLocation(ID, name), value.x, value.y);
}
void Shader::setMat4(const char* name, const glm::mat4& value) const
{
    GLint location = glGetUniformLocation(ID, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}