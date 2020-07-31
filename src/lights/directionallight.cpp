#include "directionallight.h"

void DirectionalLight::setShaderValues(const Shader* shader)
{
    shader->setVec3((name + ".ambient").c_str(), ambient);
    shader->setVec3((name + ".diffuse").c_str(), diffuse);
    shader->setVec3((name + ".specular").c_str(), specular);
    shader->setVec3((name + ".direction").c_str(), direction);
}
