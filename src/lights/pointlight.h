#pragma once


#include "../window/window.h"
#include "light.h"

class PointLight : public Light
{
public:
    glm::vec3 position;
    const float radius;
    const float linear = 0.7;
    const float quadratic = 1.8;

    PointLight(std::string structName, glm::vec3 ambient, glm::vec3 color, glm::vec3 position);
    virtual void SetShaderValues(const Shader& shader) const override;
    /*void SetDepthShaderValues(const Shader& shader) const;*/
    //void UpdateShadowTransforms(glm::mat4 shadowProjection)
    //{
    //    shadowTransforms[0] = shadowProjection * glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    //    shadowTransforms[1] = shadowProjection * glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    //    shadowTransforms[2] = shadowProjection * glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //    shadowTransforms[3] = shadowProjection * glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    //    shadowTransforms[4] = shadowProjection * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    //    shadowTransforms[5] = shadowProjection * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    //}

private:
    float getRadius() const;
};