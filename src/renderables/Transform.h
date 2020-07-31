#pragma once
#include <glm\ext\matrix_transform.hpp>
struct Transform
{
private:
    glm::mat4 m_Model;
public:
    Transform(glm::mat4& model) : m_Model(model) {}
    glm::mat4 GetModel() const
    {
        return m_Model;
    }
    glm::vec3 GetPosition() const
    {
        return glm::vec3(m_Model[3][0], m_Model[3][1], m_Model[3][2]);
    }
    glm::vec3 GetScale() const
    {
        return glm::vec3(m_Model[0][0], m_Model[1][1], m_Model[2][2]);
    }

    void SetPosition(glm::vec3 position)
    {
        m_Model = glm::translate(m_Model, position);
    }

    void SetScale(glm::vec3 scale)
    {
        m_Model = glm::scale(m_Model, scale);
    }
};