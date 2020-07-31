#pragma once
#include <glad\glad.h>
#include <vector>
#ifdef _DEBUG
#include <iostream>
#endif

class Framebuffer
{
public:
    GLuint ID;
    std::vector<GLuint> colorBuffers;
    GLuint depthBuffer;
private:
    std::vector<GLenum> m_DrawAttachments;

public:
    Framebuffer()
    {
        glGenFramebuffers(1, &ID);
    }
    static void bind(const GLuint framebufferID)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
    }
    static void bindRead(GLuint framebufferID)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferID);
    }
    static void bindDraw(GLuint framebufferID)
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferID);
    }
    void attachColorBuffers(GLsizei count, GLsizei width, GLsizei height, GLenum internalformat = GL_RGBA16F)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);

        std::vector<GLuint> buffers(count);
        glGenTextures(count, buffers.data());
        for (GLsizei i = 0; i < count; i++)
        {
            glBindTexture(GL_TEXTURE_2D, buffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i + colorBuffers.size(), GL_TEXTURE_2D, buffers[i], 0);
            m_DrawAttachments.push_back(GL_COLOR_ATTACHMENT0 + i + colorBuffers.size());
        }
        colorBuffers.insert(colorBuffers.end(), buffers.begin(), buffers.end());
        glDrawBuffers(m_DrawAttachments.size(), m_DrawAttachments.data());

#ifdef _DEBUG
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete" << std::endl;
#endif

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void attachDepthBuffer(GLsizei width, GLsizei height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);

        glGenRenderbuffers(1, &depthBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

#ifdef _DEBUG
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete" << std::endl;
#endif
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    static void blit(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
    {
        glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    }
};