#pragma once
#include <queue>

#include "../buffers/framebuffer.h"
#include "../window/window.h"
#include "../renderables/Renderable.h"
#include "../renderables/Emissive.h"

static float quadVertices[] = {
    // positions        // texture Coords
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};

class Pipeline
{
private:
    std::vector<Renderable> m_GeometryList;
    std::vector<Light*> m_LightList;
    std::vector<Emissive> m_EmissiveList;

    Framebuffer m_PingPongFBO[2];
    Framebuffer m_GBuffer;

    VertexArray m_QuadVAO;

    glm::mat4 m_Projection;
    glm::mat4 m_View;
    glm::mat4 m_Model;
public:
    Pipeline(Window& window)
    {
        Buffer* quadBuffer = new Buffer(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices);
        m_QuadVAO.addBuffer(quadBuffer, 0, 3, 5 * sizeof(float), 0);
        m_QuadVAO.addBuffer(quadBuffer, 1, 2, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        m_GBuffer.attachColorBuffers(3, window.getWidth(), window.getHeight());
        m_GBuffer.attachDepthBuffer(window.getWidth(), window.getHeight());

        m_PingPongFBO[0].attachColorBuffers(1, window.getWidth(), window.getHeight());
        m_PingPongFBO[1].attachColorBuffers(1, window.getWidth(), window.getHeight());
        //GLuint gPosition = m_GBuffer.colorBuffers[0];
        //GLuint gNormal = m_GBuffer.colorBuffers[1];
        //GLuint gAlbedoSpec = m_GBuffer.colorBuffers[2];
    }

    void UpdateProjectionView(Camera& camera, Window& window)
    {
        m_Projection = glm::perspective(glm::radians(camera.Fov), window.getAspectRatio(), 0.1f, 100.0f);
        m_View = camera.getView();
    }

    void PushToGeometryQueue(Renderable& model)
    {
        m_GeometryList.push_back(model);
    }

    void PushToLightQueue(Light* light)
    {
        m_LightList.push_back(light);
    }

    void PushToEmissiveQueue(Emissive& model)
    {
        m_EmissiveList.push_back(model);
    }

    Framebuffer GeometryPass(Window& window, Camera& camera, Shader& shader)
    {
        // 1. Geometry Pass: Render scene's geometry/color data into gbuffer
        // -----------------------------------------------------------------
        Framebuffer::bind(m_GBuffer.ID);
        Window::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        shader.use();
        shader.setMat4("projection", m_Projection);
        shader.setMat4("view", m_View);
        shader.setVec3("viewPos", camera.Position);

        for (const auto& g : m_GeometryList)
            g.Draw(shader);

        return m_GBuffer;
    }

    Framebuffer LightingPass(Framebuffer& framebuffer, Camera& camera, Shader& shader)
    {
        // 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
        // -----------------------------------------------------------------------------------------------------------------------
        Framebuffer::bind(framebuffer.ID);
        Window::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_GBuffer.colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_GBuffer.colorBuffers[1]);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_GBuffer.colorBuffers[2]);

        for (const auto& l : m_LightList)
            l->SetShaderValues(shader);

        shader.setVec3("viewPos", camera.Position);
        m_QuadVAO.bind();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        return framebuffer;
    }

    void BlitGBuffer(Framebuffer& target, Window& window)
    {
        // 2.5. copy content of geometry's depth buffer to target framebuffer's depth buffer
        // ----------------------------------------------------------------------------------
        Framebuffer::bindRead(m_GBuffer.ID);
        Framebuffer::bindDraw(target.ID);
        Framebuffer::blit(0, 0, window.getWidth(), window.getHeight(), 0, 0, window.getWidth(), window.getHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        Framebuffer::bind(0);
    }

    Framebuffer LightGeometryPass(Framebuffer& framebuffer, Shader& shader)
    {
        // 3. render lights on top of scene
        // --------------------------------
        Framebuffer::bind(framebuffer.ID);
        shader.use();
        shader.setMat4("projection", m_Projection);
        shader.setMat4("view", m_View);
        for (const auto& e : m_EmissiveList)
            e.Draw(shader);
        return framebuffer;
    }

    Framebuffer BlurPass(Framebuffer& framebuffer, Shader& shader)
    {
        // 3.5. Blur bright areas
        // ----------------------
        Framebuffer::bind(framebuffer.ID);
        bool horizontal = true, first_iteration = true;
        int amount = 10;
        shader.use();
        m_QuadVAO.bind();
        for (GLuint i = 0; i < amount; i++)
        {
            Framebuffer::bind(m_PingPongFBO[horizontal].ID);
            shader.setInt("horizontal", horizontal);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? framebuffer.colorBuffers[1] : m_PingPongFBO[!horizontal].colorBuffers[0]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindVertexArray(0);
        return m_PingPongFBO[!horizontal];
    }

    void FinalPass(std::vector<GLuint>& textures, Shader& shader)
    {
        // 4. Perform Postprocessing (HDR, Bloom)
        // --------------------------------------
        Framebuffer::bind(0);
        Window::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        for (int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0+i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
        }
        m_QuadVAO.bind();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

    }

};