#pragma once
#include <vector>
#include <GLFW/glfw3.h>
#include "buffer.h"

class VertexArray
{
private:
    GLuint m_ArrayID;
    std::vector<Buffer*> m_Buffers;
public:
    VertexArray()
    {
        glGenVertexArrays(1, &m_ArrayID);
    }
    ~VertexArray()
    {
        for (int i = 0; i < m_Buffers.size(); i++)
        {
            m_Buffers[i]->uses--;
            if (m_Buffers[i]->uses == 0)
            {
                delete m_Buffers[i];
            }
        }

        glDeleteVertexArrays(1, &m_ArrayID);
    }

    void addBuffer(Buffer* buffer, GLuint index, GLuint componentCount, GLsizei stride = 0, const void* start = 0)
    {
        buffer->uses++;
        m_Buffers.push_back(buffer);

        bind();
        buffer->bind();

        glVertexAttribPointer(index, componentCount, GL_FLOAT, GL_FALSE, stride, start);
        glEnableVertexAttribArray(index);

        buffer->unbind();
        VertexArray::unbind();
    }
    inline void bind() const { glBindVertexArray(m_ArrayID); }
    static inline void unbind() { glBindVertexArray(0); }
};