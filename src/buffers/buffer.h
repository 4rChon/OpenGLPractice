#pragma once

#include <glad\glad.h>
#include <GLFW\glfw3.h>

class Buffer
{
public:
    GLuint uses = 0;
private:
    GLuint m_BufferID;
    GLenum m_BufferType;
public:
    Buffer(GLenum bufferType, GLsizeiptr size, const void* data)
        :m_BufferType(bufferType)
    {
        glGenBuffers(1, &m_BufferID);
        glBindBuffer(m_BufferType, m_BufferID);
        glBufferData(m_BufferType, size, data, GL_STATIC_DRAW);
        glBindBuffer(m_BufferType, 0);
    }
    ~Buffer()
    {
        glDeleteBuffers(1, &m_BufferID);
    }
    inline void bind() const { glBindBuffer(m_BufferType, m_BufferID); }
    inline void bindBufferRange(GLuint index, GLintptr offset, GLsizeiptr size) const 
    {
        glBindBufferRange(m_BufferType, index, m_BufferID, offset, size);
    }
    inline void setBufferSubData(GLintptr offset, GLsizeiptr size, const void* data) const
    {
        glBindBuffer(m_BufferType, m_BufferID);
        glBufferSubData(m_BufferType, offset, size, data);
        glBindBuffer(m_BufferType, 0);
    }
    inline void unbind() const { glBindBuffer(m_BufferType, 0); }
};
