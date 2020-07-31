#pragma once

#include <glm\glm.hpp>
#include <GLFW\glfw3.h>

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float FOV = 45.0f;

class Camera
{
public:
    glm::vec3 Position, Front, Up, Right, WorldUp;
    float Yaw, Pitch;
    float MoveSpeed, MouseSensitivity, Fov;
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH, float fov = FOV);
    void update();
    glm::mat4 getView() const;
    glm::mat4 getReverseView() const;
    void translate(glm::vec3 delta) { Position += delta; }
    void zoom(float delta);
    void rotateYaw(float delta);
    void rotatePitch(float delta);
};