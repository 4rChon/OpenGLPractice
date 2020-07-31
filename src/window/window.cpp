#include <iostream>

#include "window.h"

Window::Window(const char* title, int width, int height)
    : m_Title(title), m_Width(width), m_Height(height)
{
    if (!init())
    {
        glfwTerminate();
    }
}

Window::~Window()
{
    glfwTerminate();
}

bool Window::init()
{
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(m_Width, m_Height, m_Title, NULL, NULL);
    if (m_Window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        return false;
    }
    glfwMakeContextCurrent(m_Window);
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    glfwSetCursorPosCallback(m_Window, mouse_callback);
    //glfwSetScrollCallback(m_Window, scroll_callback);
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_FRAMEBUFFER_SRGB);
    //glEnable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //glFrontFace(GL_CCW);
    //glCullFace(GL_FRONT);

    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    m_Mouse.x = m_Width / 2;
    m_Mouse.y = m_Height / 2;
    
    return true;
}

void Window::close() const
{
    glfwSetWindowShouldClose(m_Window, true);
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_Window);
}

bool Window::isKeyPressed(int keycode) const
{
    return glfwGetKey(m_Window, keycode) == GLFW_PRESS;
}

bool Window::isKeyReleased(int keycode) const
{
    return glfwGetKey(m_Window, keycode) == GLFW_RELEASE;
}

void Window::update() const
{
#if _DEBUG
    check_errors();
#endif
    glfwPollEvents();
    glfwSwapBuffers(m_Window);
}

bool Window::check_errors()
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::string error_string;
        switch (error)
        {
        case GL_INVALID_ENUM: error_string = "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE: error_string = "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: error_string = "GL_INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW: error_string = "GL_STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW: error_string = "GL_STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY: error_string = "GL_OUT_OF_MEMORY"; break;
        }
        std::cout << "OpenGL Error: " << error << " **" << error_string << "**" << std::endl;
        return true;
    }

    return false;
}

void Window::clear(GLbitfield mask)
{
    glClear(mask);
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    Window* win = (Window*)glfwGetWindowUserPointer(window);
    win->m_Width = width;
    win->m_Height = height;
    glViewport(0, 0, width, height);
}

void Window::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    Window* win = (Window*)glfwGetWindowUserPointer(window);
    Camera* camera = win->getCamera(0);
    if (win->m_Mouse.firstMouse)
    {
        win->m_Mouse.x = xpos;
        win->m_Mouse.y = ypos;
        win->m_Mouse.firstMouse = false;
    }
    win->m_Mouse.deltaX = xpos - win->m_Mouse.x;
    win->m_Mouse.deltaY = win->m_Mouse.y - ypos;

    win->m_Mouse.x = xpos;
    win->m_Mouse.y = ypos;

    win->m_Mouse.deltaX *= camera->MouseSensitivity;
    win->m_Mouse.deltaY *= camera->MouseSensitivity;

    camera->rotateYaw(win->m_Mouse.deltaX);
    camera->rotatePitch(win->m_Mouse.deltaY);
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Window* win = (Window*)glfwGetWindowUserPointer(window);
    Camera* camera = win->getCamera(0);
    camera->zoom(yoffset);
}