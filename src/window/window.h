#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "../camera/camera.h"

struct Mouse
{
    double x = 0, y = 0;
    float deltaX = 0.0f, deltaY = 0.0f;
    bool firstMouse = true;
};

class Window
{
private:
    GLFWwindow* m_Window;
    const char* m_Title;
    int m_Width, m_Height;
    Mouse m_Mouse;
    std::vector<Camera*> cameras;
public:
    Window(const char* title, int width, int height);
    ~Window();
    static bool check_errors();
    static void clear(GLbitfield mask);
    void close() const;
    bool shouldClose() const;
    void update() const;
    bool isKeyPressed(int keycode) const;
    bool isKeyReleased(int keycode) const;
    inline int getWidth() const { return m_Width; }
    inline int getHeight() const { return m_Height; }
    inline float getAspectRatio() const { return (float)m_Width / (float)m_Height; }
    inline Mouse& getMouse() { return m_Mouse; }
    inline void addCamera(Camera* camera) { cameras.push_back(camera); }
    inline Camera* getCamera(int index) { return cameras[index]; }
private:
    bool init();
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};