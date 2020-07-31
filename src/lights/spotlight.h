#pragma once
#include <string>
#include <glm/glm.hpp>
#include "../shaders/shader.h"
struct SpotLight
{
    const std::string name;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 position;
    glm::vec3 direction;
    float k1;
    float k2;
    float k3;
    float cutOff;
    float outerCutOff;

    SpotLight(const char* structName, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 position, glm::vec3 direction, float k1, float k2, float k3, float cutOff, float outerCutoff)
        : name(structName), ambient(ambient), diffuse(diffuse), specular(specular), position(position), direction(direction), k1(k1), k2(k2), k3(k3), cutOff(cutOff), outerCutOff(outerCutoff) {}
    void setShaderValues(const Shader* shader);
};