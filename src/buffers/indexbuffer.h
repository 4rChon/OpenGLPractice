#pragma once

#include <glad\glad.h>
#include <GLFW\glfw3.h>

class IndexBuffer
{
private:
    GLuint m_BufferID;
    GLuint m_Count;
public:
    IndexBuffer(const GLushort* data, GLsizei count)
        : m_Count(count)
    {
        glGenBuffers(1, &m_BufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLushort), data, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    ~IndexBuffer()
    {
        glDeleteBuffers(1, &m_BufferID);
    }
    inline void bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID); }
    static inline void unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
    inline GLuint getCount() const { return m_Count; }
};
