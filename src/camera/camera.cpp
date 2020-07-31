#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float fov)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MoveSpeed(SPEED), MouseSensitivity(SENSITIVITY), Position(position), WorldUp(up), Yaw(yaw), Pitch(pitch), Fov(fov)
{
    update();
}

void Camera::update()
{
    Front = glm::normalize(glm::vec3(
        cos(glm::radians(Yaw)) * cos(glm::radians(Pitch)),
        sin(glm::radians(Pitch)),
        sin(glm::radians(Yaw)) * cos(glm::radians(Pitch)))
    );
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

glm::mat4 Camera::getView() const
{
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::getReverseView() const
{
    return glm::lookAt(Position, Position - Front, Up);
}

void Camera::zoom(float delta)
{
    Fov -= delta;
    if (Fov < 1.0f)
        Fov = 1.0f;
    if (Fov > 45.0f)
        Fov = 45.0f;
}
void Camera::rotateYaw(float delta)
{
    Yaw += delta;
}
void Camera::rotatePitch(float delta)
{
    Pitch += delta;

    if (Pitch > 89.0f)
        Pitch = 89.0f;
    if (Pitch < -89.0f)
        Pitch = -89.0f;
}