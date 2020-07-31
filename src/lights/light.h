#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include "../shaders/shader.h"
class Light
{
public:
    const std::string name;
    const glm::vec3 ambient;
    const glm::vec3 color;
protected:
    /*glm::uvec2 shadow_resolution;
    std::vector<glm::mat4> shadowTransforms;
    GLuint shadowMapFBO;
    GLuint depthCubemap;*/
public:
    Light(std::string structName, glm::vec3 ambient, glm::vec3 color)
        :name(structName), ambient(ambient), color(color)//, shadow_resolution(shadow_resolution)
    {}
    //void BindShadowBuffer() const
    //{
    //    glViewport(0, 0, shadow_resolution.x, shadow_resolution.y);
    //    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    //    glClear(GL_DEPTH_BUFFER_BIT);
    //}

    /*virtual void UpdateShadowTransforms(glm::mat4 shadowProjection) = 0;*/
    /*virtual void SetDepthShaderValues(const Shader& shader) const = 0;*/
    virtual void SetShaderValues(const Shader& shader) const = 0;
};