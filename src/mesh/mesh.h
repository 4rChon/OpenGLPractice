#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <gl/GL.h>
#include <string>
#include <vector>
#include "../shaders/shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec3 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Texture {
    GLuint id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    GLuint VAO, VBO, EBO;

    Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);
    void initDraw(Shader& shader) const;
    void draw(Shader& shader) const;
private:
    void setupMesh();
};