#include "pointlight.h"

PointLight::PointLight(std::string structName, glm::vec3 ambient, glm::vec3 color, glm::vec3 position)
    : Light(structName, ambient, color), position(position), radius(getRadius())
{
    //shadowTransforms.resize(6);
    //glGenFramebuffers(1, &shadowMapFBO);
    //glGenTextures(1, &depthCubemap);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    //for (GLuint i = 0; i < 6; i++)
    //{
    //    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, shadow_resolution.x, shadow_resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    //}
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    //glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    //glDrawBuffer(GL_NONE);
    //glReadBuffer(GL_NONE);
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float PointLight::getRadius() const
{
    float I = std::fmaxf(std::fmaxf(color.r * 0.2126f, color.g * 0.7152f), color.b * 0.0722f);
    return std::sqrtf(4.0f * I * (256.0f / 5.0f)) / 2.0f;
}

void PointLight::SetShaderValues(const Shader& shader) const
{
    shader.setVec3((name + ".Ambient").c_str(), ambient);
    shader.setVec3((name + ".Color").c_str(), color);
    shader.setVec3((name + ".Position").c_str(), position);
    shader.setFloat((name + ".Linear").c_str(), linear);
    shader.setFloat((name + ".Quadratic").c_str(), quadratic);
    //shader.setInt((name + ".ShadowMap").c_str(), depthCubemap);
    //glActiveTexture(GL_TEXTURE0 + depthCubemap);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
}

//void PointLight::SetDepthShaderValues(const Shader& shader) const
//{
//    shader.setVec3("lightPos", position);
//    for (int i = 0; i < shadowTransforms.size(); i++)
//    {
//        shader.setMat4(
//            ("shadowMatrices[" + std::to_string(i) + "]").c_str(),
//            shadowTransforms[i]
//        );
//    }
//}