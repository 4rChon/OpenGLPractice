#include "model.h"
#include <iostream>
#include "../utils/stb_image.h"
#include "../utils/fileutils.h"
#if _DEBUG
#include "../window/window.h"
#endif

void Model::Draw(Shader& shader) const
{
    for (GLuint i = 0; i < meshes.size(); i++)
        meshes[i].draw(shader);
}

void Model::InstancedDraw(Shader& shader, int amount)
{
    for (GLuint i = 0; i < meshes.size(); i++)
    {
        meshes[i].initDraw(shader);
        glBindVertexArray(meshes[i].VAO);
        glDrawElementsInstanced(
            GL_TRIANGLES, meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount
        );
    }
}

void Model::loadModel(std::string path)
{
#ifdef _DEBUG
    std::cout << "Loading model at " + path << std::endl;
#endif
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
#ifdef _DEBUG
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
#endif
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
#ifdef _DEBUG
    bool error = Window::check_errors();
    if (error)
        std::cout << "Error loading model!" << std::endl;
    else
        std::cout << "Model loaded with no errors" << std::endl;
#endif
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    for (GLuint i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (GLuint i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    for (GLuint i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        aiVector3D position = mesh->mVertices[i];
        vertex.Position = glm::vec3(position.x, position.y, position.z);

        aiVector3D normal = mesh->mNormals[i];
        vertex.Normal = glm::vec3(normal.x, normal.y, normal.z);

        if (mesh->mTextureCoords[0])
        {
            aiVector3D texcoords = mesh->mTextureCoords[0][i];
            vertex.TexCoords = glm::vec3(texcoords.x, texcoords.y, texcoords.z);
        }

        aiVector3D tangent = mesh->mTangents[i];
        vertex.Tangent = glm::vec3(tangent.x, tangent.y, tangent.z);

        aiVector3D bitangent = mesh->mBitangents[i];
        vertex.Bitangent = glm::vec3(bitangent.x, bitangent.y, bitangent.z);
        
        vertices.push_back(vertex);
    }

    for (GLuint i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (GLuint j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (GLuint j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            Texture texture;
            const char* c_str = str.C_Str();
            texture.id = TextureFromFile(c_str, directory);
            texture.type = typeName;
            texture.path = std::string(c_str);
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }

    return textures;
}