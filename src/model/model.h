#pragma once
#include "../shaders/shader.h"
#include <vector>
#include <string>
#include "../mesh/mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
    std::vector<Mesh> meshes;
private:
    std::vector<Texture> textures_loaded;
    std::string directory;
public:
    Model(const char* path) { loadModel(path); }
    void InstancedDraw(Shader& shader, int amount);
    void Draw(Shader& shader) const;
private:
    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};
