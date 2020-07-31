#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>

#include "src/window/window.h"
#include "src/shaders/shader.h"
#include "src/camera/camera.h"
#include "src/buffers/vertexarray.h"
#include "src/lights/pointlight.h"
#include "src/lights/directionallight.h"
#include "src/lights/spotlight.h"
#include "src/model/model.h"
#include "src/utils/stb_image.h"
#include "src/utils/fileutils.h"
#include "src/buffers/framebuffer.h"
#include "src/pipeline/pipeline.h"
#include "src/renderables/Emissive.h"

static const GLuint POINT_LIGHTS = 16;

static const GLuint SCR_WIDTH = 1920;
static const GLuint SCR_HEIGHT = 1024;

static const GLuint SHADOW_WIDTH = 1024;
static const GLuint SHADOW_HEIGHT = 1024;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(Window& window, Camera& camera);
GLuint loadTexture(char const* path);
void initCubeVAO(VertexArray& cubeVAO);
void initPlaneVAO(VertexArray& planeVAO);
void initQuadVAO(VertexArray& quadVAO);
std::vector<PointLight> initLights();
void drawPlane(const Shader& shader, const VertexArray& VAO, GLuint texture = 0);
void drawSkybox(const Shader& shader, const VertexArray& VAO, GLuint texture = 0);
void drawCubes(const Shader& shader, std::vector<glm::vec3> objectPositions, const VertexArray& VAO, GLuint diffuse, GLuint normal, GLuint specular, GLuint height);
void drawQuad(const VertexArray& VAO, GLuint texture = 0);
void drawWindows(const Shader& shader, const VertexArray& VAO, GLuint texture, std::map<float, glm::vec3>& sorted);

GLuint loadSkybox(std::vector<std::string> faces);

int main()
{
    // Init Cindow
    // -----------
    Window window("LearnOpenGL", SCR_WIDTH, SCR_HEIGHT);
    // -----------

    // Init Camera
    // -----------
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
    window.addCamera(&camera);
    // -----------

    // Init Shaders
    // ------------
    //Shader depthShader;
    //depthShader.attachShader("src/shaders/Depthmap.vert", GL_VERTEX_SHADER);
    //depthShader.attachShader("src/shaders/Depthmap.geom", GL_GEOMETRY_SHADER);
    //depthShader.attachShader("src/shaders/Depthmap.frag", GL_FRAGMENT_SHADER);
    //depthShader.linkProgram();
    GLuint geometryShaders[2] = {
        Shader::createShader("src/shaders/GBuffer.vert", GL_VERTEX_SHADER),
        Shader::createShader("src/shaders/GBuffer.frag", GL_FRAGMENT_SHADER)
    };
    Shader shaderGeometryPass(2, geometryShaders);

    GLuint lightingShaders[2] = {
        Shader::createShader("src/shaders/DeferredShading.vert", GL_VERTEX_SHADER),
        Shader::createShader("src/shaders/DeferredShading.frag", GL_FRAGMENT_SHADER)
    };
    Shader shaderLightingPass(2, lightingShaders);

    GLuint lightBoxShaders[2] = {
        Shader::createShader("src/shaders/DeferredLightBox.vert", GL_VERTEX_SHADER),
        Shader::createShader("src/shaders/DeferredLightBox.frag", GL_FRAGMENT_SHADER)
    };
    Shader shaderLightBox(2, lightBoxShaders);

    GLuint postProcessingShaders[2] = {
        Shader::createShader("src/shaders/PostProcessing.vert", GL_VERTEX_SHADER),
        Shader::createShader("src/shaders/PostProcessing.frag", GL_FRAGMENT_SHADER)
    };
    Shader shaderPostProcessing(2, postProcessingShaders);

    GLuint blurShaders[2] = {
        Shader::createShader("src/shaders/Blur.vert", GL_VERTEX_SHADER),
        Shader::createShader("src/shaders/Blur.frag", GL_FRAGMENT_SHADER)
    };
    Shader shaderBlur(2, blurShaders);
    // ------------

    // Init Textures
    // -------------
    //GLuint brickNormal = TextureFromFile("brickwall_normal.jpg", "Resources");
    //GLuint brick = TextureFromFile("brickwall.jpg", "Resources", true);
    //GLuint brick_normal = TextureFromFile("brickwall_normal.jpg", "Resources");
    //GLuint brick_specular = TextureFromFile("brickwall_specular.jpg", "Resources");
    //GLuint brick_diffuse = TextureFromFile("brickwall.jpg", "Resources");
    //GLuint brick_height = TextureFromFile("brickwall_disp.jpg", "Resources");
    Model cube("Resources/cube/cube.obj");
    Model backpack("Resources/cube/cube.obj");
    std::vector<glm::vec3> objectPositions = {
        glm::vec3(-3.0,  -0.5, -3.0),
        glm::vec3(0.0,  -0.5, -3.0),
        glm::vec3(3.0,  -0.5, -3.0),
        glm::vec3(-3.0,  -0.5,  0.0),
        glm::vec3(0.0,  -0.5,  0.0),
        glm::vec3(3.0,  -0.5,  0.0),
        glm::vec3(-3.0,  -0.5,  3.0),
        glm::vec3(0.0,  -0.5,  3.0),
        glm::vec3(3.0,  -0.5,  3.0)
    };
    std::vector<Renderable> renderables;
    for (int i = 0; i < objectPositions.size(); i++)
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0), objectPositions[i]);
        Transform transform(model);
        Renderable renderable(backpack, transform);
        renderables.push_back(renderable);
    }

    // -------------
    // Init Framebuffers
    // -----------------
    Pipeline pipeline(window);
    Framebuffer deferredFBO;
    deferredFBO.attachColorBuffers(2, window.getWidth(), window.getHeight());
    deferredFBO.attachDepthBuffer(window.getWidth(), window.getHeight());
    // -----------------

    // Init VAOs
    // ---------
    VertexArray cubeVAO, quadVAO, planeVAO;
    initCubeVAO(cubeVAO);
    initQuadVAO(quadVAO);
    initPlaneVAO(planeVAO);
    // ---------

    // Init Lights
    // -----------
    std::vector<PointLight> lights = initLights();
    std::vector<Emissive> emissives;
    for (int i = 0; i < lights.size(); i++)
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0), lights[i].position);
        Transform transform(model);
        transform.SetScale(glm::vec3(0.25f));
        Emissive emissive(cube, new PointLight(lights[i]), transform);
        emissives.push_back(emissive);
    }
    // ---------------------
    // Init Render Variables
    // ---------------------
    float near_plane = 1.0f;
    float far_plane = 128.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
    // ---------------------

    // Init Shader Uniforms
    // --------------------
    shaderLightingPass.use();
    shaderLightingPass.setInt("gPosition", 0);
    shaderLightingPass.setInt("gNormal", 1);
    shaderLightingPass.setInt("gAlbedoSpec", 2);

    shaderGeometryPass.use();
    shaderGeometryPass.setFloat("minLayers", 16.0f);
    shaderGeometryPass.setFloat("maxLayers", 64.0f);
    shaderGeometryPass.setFloat("heightScale", 0.025f);
    shaderGeometryPass.setInt("material.texture_diffuse1", 0);
    shaderGeometryPass.setInt("material.texture_normal1", 1);
    shaderGeometryPass.setInt("material.texture_specular1", 2);
    shaderGeometryPass.setInt("material.texture_height1", 3);

    //shader.setFloat("far_plane", far_plane);

    shaderPostProcessing.use();
    shaderPostProcessing.setInt("scene_color", 0);
    shaderPostProcessing.setInt("scene_bloom", 1);
    shaderPostProcessing.setFloat("exposure", 0.01f);

    shaderBlur.use();
    shaderBlur.setInt("image", 0);

    //Shader::use(depthShader);
    //depthShader.setFloat("far_plane", far_plane);
    // --------------------
    for (int i = 0; i < renderables.size(); i++)
    {
        pipeline.PushToGeometryQueue(renderables[i]);
    }

    for (int i = 0; i < lights.size(); i++)
    {
        pipeline.PushToLightQueue(&lights[i]);
    }

    for (int i = 0; i < emissives.size(); i++)
    {
        pipeline.PushToEmissiveQueue(emissives[i]);
    }

    bool useNormal = true;
    while (!window.shouldClose())
    {
        //for (int i = 0; i < )
        // Per-frame Time Logic
        // --------------------
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // --------------------
        
        // Process Input
        // -------------
        processInput(window, camera);
        if (window.isKeyPressed(GLFW_KEY_T))
        {
            useNormal = !useNormal;
            shaderGeometryPass.use();
            shaderGeometryPass.setBool("useNormal", useNormal);
        }

        // -------------
        pipeline.UpdateProjectionView(camera, window);
        // Render Stage
        // ------------
        Window::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        pipeline.GeometryPass(window, camera, shaderGeometryPass);
        pipeline.LightingPass(deferredFBO, camera, shaderLightingPass);
        pipeline.BlitGBuffer(deferredFBO, window);
        pipeline.LightGeometryPass(deferredFBO, shaderLightBox);
        auto buffer = pipeline.BlurPass(deferredFBO, shaderBlur);
        std::vector<GLuint> textures = { deferredFBO.colorBuffers[0], buffer.colorBuffers[0] };
        pipeline.FinalPass(textures, shaderPostProcessing);
        //// 1. Render depth map
        //// ------------------
        //Shader::use(depthShader);
        //for (int i = 0; i < POINT_LIGHTS; i++)
        //{
        //    lights[i].updateShadowTransforms(shadowProj);
        //    lights[i].bindShadowBuffer();
        //        lights[i].setDepthShaderValues(depthShader);
        //        drawCubes(depthShader, cubeVAO);
        //        drawPlane(depthShader, planeVAO);
        //    Framebuffer::unbind();
        //}
        ////Window::check_errors();
        //// 2. Render to colorbuffer
        //// ------------------------
        //hdrFBO.bind();
        //glViewport(0, 0, window.getWidth(), window.getHeight());
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), window.getAspectRatio(), 0.1f, 100.0f);
        //glm::mat4 view = camera.getView();

        //Shader::use(shader);
        //shader.setMat4("projection", projection);
        //shader.setMat4("view", view);
        //shader.setVec3("viewPos", camera.Position);
        //for (int i = 0; i < POINT_LIGHTS; i++)
        //{
        //    lights[i].setShaderValues(shader);
        //}
        //shader.setBool("normal_map", true);
        //shader.setBool("height_map", true);
        //drawCubes(shader, cubeVAO, brick_diffuse, brick_normal, brick_height);
        //shader.setBool("normal_map", false);
        //shader.setBool("height_map", false);
        //drawPlane(shader, planeVAO, brick_diffuse);

        //Shader::use(lightingShader);
        //lightingShader.setMat4("projection", projection);
        //lightingShader.setMat4("view", view);
        //cubeVAO.bind();
        //for (int i = 0; i < POINT_LIGHTS; i++)
        //{
        //    lights[i].draw(lightingShader);
        //}
        ////Window::check_errors();
        //// 3. Blur bright fragments with two-pass Gaussian Blur
        //// ----------------------------------------------------
        //bool horizontal = true, first_iteration = true;
        //int amount = 10;
        //Shader::use(blurShader);
        //for (GLuint i = 0; i < amount; i++)
        //{
        //    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal].framebufferID);
        //    blurShader.setInt("horizontal", horizontal);
        //    glActiveTexture(GL_TEXTURE0);
        //    glBindTexture(GL_TEXTURE_2D, first_iteration ? hdrFBO.colorBuffers[1] : pingpongFBO[!horizontal].colorBuffers[0]);
        //    drawQuad(quadVAO);
        //    horizontal = !horizontal;
        //    if (first_iteration)
        //        first_iteration = false;
        //}
        ////Window::check_errors();
        //// 4. Render color buffer to 2D screen-filling quad with tone mapping shader
        //// -------------------------------------------------------------------------
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Shader::use(finalShader);
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, hdrFBO.colorBuffers[0]);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, pingpongFBO[!horizontal].colorBuffers[0]);
        //drawQuad(quadVAO);

        // check and call events and swap the buffers
        camera.update();
        window.update();
    }
    return 0;
}

std::vector<PointLight> initLights()
{
    srand(13);
    std::vector<PointLight> lights;
    for (GLsizei i = 0; i < POINT_LIGHTS; i++)
    {
        float xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
        float yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;
        float zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;

        float rColor = rand() % 100;
        float gColor = rand() % 100;
        float bColor = rand() % 100;

        lights.push_back(PointLight(
            "pointLight[" + std::to_string(i) + "]",
            glm::vec3(rColor, gColor, bColor) * 0.0001f,
            glm::vec3(rColor, gColor, bColor),
            glm::vec3(xPos, yPos, zPos)
          //  glm::uvec2(SHADOW_WIDTH, SHADOW_HEIGHT)
        ));
    }

    return lights;
}

void initCubeVAO(VertexArray& cubeVAO)
{
    float cubeVertices[] = {
        // back face
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
        // front face
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        // right face
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
        // bottom face
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
        // top face
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
         1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
         1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left      
    };

    Buffer* cubeBuffer = new Buffer(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices);
    cubeVAO.addBuffer(cubeBuffer, 0, 3, 8 * sizeof(float), 0);
    cubeVAO.addBuffer(cubeBuffer, 1, 3, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    cubeVAO.addBuffer(cubeBuffer, 2, 2, 8 * sizeof(float), (void*)(6 * sizeof(float)));
}

void initPlaneVAO(VertexArray& planeVAO)
{
    float planeVertices[] = {
        // positions            // normals         // texcoords
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
        -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,

        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
         25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f
    };

    Buffer* planeBuffer = new Buffer(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices);
    planeVAO.addBuffer(planeBuffer, 0, 3, 8 * sizeof(float), 0);
    planeVAO.addBuffer(planeBuffer, 1, 3, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    planeVAO.addBuffer(planeBuffer, 2, 2, 8 * sizeof(float), (void*)(6 * sizeof(float)));
}

void initQuadVAO(VertexArray& quadVAO)
{
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    Buffer* quadBuffer = new Buffer(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices);
    quadVAO.addBuffer(quadBuffer, 0, 3, 5 * sizeof(float), 0);
    quadVAO.addBuffer(quadBuffer, 1, 2, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void drawPlane(const Shader& shader, const VertexArray& VAO, GLuint texture)
{
    if (texture != 0)
    {
        glActiveTexture(GL_TEXTURE0 + texture);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    //if (normal != 0)
    //{
    //    glActiveTexture(GL_TEXTURE0 + normal);
    //    glBindTexture(GL_TEXTURE_2D, normal);
    //}

    //if (height != 0)
    //{
    //    glActiveTexture(GL_TEXTURE0 + height);
    //    glBindTexture(GL_TEXTURE_2D, height);
    //}
    VAO.bind();
    glm::mat4 model = glm::mat4(1.0);
    model = glm::translate(model, glm::vec3(0.0f, -3.5f, 0.0f));
    shader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    VertexArray::unbind();
}

void drawQuad(const VertexArray& VAO, GLuint texture)
{
    VAO.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    VertexArray::unbind();
}

void drawSkybox(const Shader& shader, const VertexArray& VAO, GLuint texture)
{
    if (texture != 0)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    }
    VAO.bind();
    shader.setMat4("model", glm::mat4(1.0));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    VertexArray::unbind();
}

void drawCubes(const Shader& shader, std::vector<glm::vec3> objectPositions, const VertexArray& VAO, GLuint diffuse, GLuint normal, GLuint specular, GLuint height)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, specular);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, height);
    VAO.bind();
    glm::mat4 model = glm::mat4(1.0f);
    for (GLsizei i = 0; i < objectPositions.size(); i++)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, objectPositions[i]);
        model = glm::scale(model, glm::vec3(0.5f));
        shader.setMat4("model", model);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    VertexArray::unbind();
}
void drawWindows(const Shader& shader, const VertexArray& VAO, GLuint texture, std::map<float, glm::vec3>& sorted)
{
    VAO.bind();
    if (texture != 0)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
    {
        glm::mat4 model = glm::mat4(1.0);
        model = glm::translate(model, it->second);
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    VertexArray::unbind();
}

void processInput(Window& window, Camera& camera)
{
    if (window.isKeyPressed(GLFW_KEY_ESCAPE))
    {
        window.close();
        return;
    }
    float cameraSpeed = camera.MoveSpeed * deltaTime;
    if (window.isKeyPressed(GLFW_KEY_LEFT_SHIFT))
        cameraSpeed *= 8;
    if (window.isKeyPressed(GLFW_KEY_W))
        camera.translate(cameraSpeed * camera.Front);
    if (window.isKeyPressed(GLFW_KEY_S))
        camera.translate(cameraSpeed * -camera.Front);
    if (window.isKeyPressed(GLFW_KEY_A))
        camera.translate(cameraSpeed * -camera.Right);
    if (window.isKeyPressed(GLFW_KEY_D))
        camera.translate(cameraSpeed * camera.Right);
    if (window.isKeyPressed(GLFW_KEY_SPACE))
        camera.translate(cameraSpeed * camera.Up);
    if (window.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
        camera.translate(cameraSpeed * -camera.Up);
}

GLuint loadSkybox(std::vector<std::string> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format = GL_RED;
            if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
        }
        stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return textureID;
}

GLuint loadTexture(char const* path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }

    stbi_image_free(data);
    return textureID;
}
