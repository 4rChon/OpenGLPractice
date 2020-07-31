#pragma once
#include <glm/glm.hpp>
#include "../shaders/shader.h"
#include <string>

struct DirectionalLight
{
    const std::string name;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 direction;
    DirectionalLight(const char* structName, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 direction)
        : name(structName), ambient(ambient), diffuse(diffuse), specular(specular), direction(direction) {}
    void setShaderValues(const Shader* shader);
};