#include "spotlight.h"

void SpotLight::setShaderValues(const Shader* shader)
{
    shader->setVec3((name + ".ambient").c_str(), ambient);
    shader->setVec3((name + ".diffuse").c_str(), diffuse);
    shader->setVec3((name + ".specular").c_str(), specular);
    shader->setVec3((name + ".position").c_str(), position);
    shader->setVec3((name + ".direction").c_str(), direction);
    shader->setFloat((name + ".constant").c_str(), k1);
    shader->setFloat((name + ".linear").c_str(), k2);
    shader->setFloat((name + ".quadratic").c_str(), k3);
    shader->setFloat((name + ".cutOff").c_str(), cutOff);
    shader->setFloat((name + ".outerCutOff").c_str(), outerCutOff);
}
