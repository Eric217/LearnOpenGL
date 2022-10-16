//
//  Model.cpp
//  06-model
//
//  Created by Eric on 2022/10/10.
//

#include "Model.hpp"

#define DIFFUSE_TEXTURE_PREFIX "texture_diffuse"
#define SPECULAR_TEXTURE_PREFIX "texture_specular"

void Mesh::setup() {
    GLuint buffers[2];
    glGenBuffers(2, buffers);
    vbo = buffers[0];
    ebo = buffers[1];
    glGenVertexArrays(1, &vao);
    
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData (GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    
    glVertexAttribPointer (0, 3, GL_FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, position));
    glVertexAttribPointer (1, 3, GL_FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    glVertexAttribPointer (2, 2, GL_FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, texCoor));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
}

Mesh::Mesh(Mesh &&mesh) {
    vao = mesh.vao;
    vbo = mesh.vbo;
    ebo = mesh.ebo;
    vertices = std::move(mesh.vertices);
    indices = std::move(mesh.indices);
    textures = std::move(mesh.textures);
}

Mesh::~Mesh() {
}

static std::string samplerNamePrefix = "material.";
static std::string useSamplerNamePrefix = "material.use_";

void Mesh::draw(const Shader &shader) const {
    glBindVertexArray(vao);

    // 每个纹理绑到一个槽位上，按照类型绑到不同的 uniform sampler 上
    auto diffNr = 0, specNr = 0;
    
    for (auto i = 0; i < textures.size(); i++) {
        textures[i].use(i);
        // material.texture_diffuse0
        if (textures[i].type == DIFFUSE_TEXTURE_PREFIX) {
            auto s = textures[i].type + std::to_string(diffNr++);
            shader.setInt(samplerNamePrefix + s, i);
            shader.setBool(useSamplerNamePrefix + s, true);
        } else if (textures[i].type == SPECULAR_TEXTURE_PREFIX) {
            auto s = textures[i].type + std::to_string(specNr++);
            shader.setInt(samplerNamePrefix + s, i);
            shader.setBool(useSamplerNamePrefix + s, true);
        } else {
            assert(false); // 未处理!
        }
    }
    glDrawElements (GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

// MARK: - Model
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

std::vector<Texture> Model::loadTexture(const aiMaterial& mat, aiTextureType type, const std::string& namePrefix) {
    std::vector<Texture> textures;
    
    auto dCount = mat.GetTextureCount(type);

    for (int j = 0; j < dCount; j++) {
        aiString p;
        mat.GetTexture(type, j, &p);
        if (!p.length) {
            continue;
        }
        auto path = directory + p.C_Str();
        auto t = Texture::load(path, namePrefix);
        textures.push_back(std::move(t));
    }
    return textures;
}

void Model::processNode(const aiScene* scene, aiNode *node, Node &mynode) {
    if (!node) {
        return;
    }

    for (int i = 0; i < node->mNumMeshes; i++) {
        const auto meshIdx = node->mMeshes[i];
        const auto mesh = scene->mMeshes[meshIdx];
        
        assert(mesh->HasNormals());
        std::vector<unsigned int> indices;
        std::vector<Vertex> vertices;
        std::vector<Texture> textures;
        
        for (int j = 0; j < mesh->mNumVertices; j++) {
            Vertex v;
            v.position = glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
            v.normal = glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);
            if (mesh->mTextureCoords && mesh->mTextureCoords[0]) {
                const auto& c = mesh->mTextureCoords[0][j];
                v.texCoor = glm::vec2(c.x, c.y);
            } else {
                v.texCoor = glm::vec2(0);
            }
            vertices.push_back(std::move(v));
        }
        for (int j = 0; j < mesh->mNumFaces; j++) {
            const auto& face = mesh->mFaces[j];
            assert(face.mNumIndices == 3);
            for (int k = 0; k < face.mNumIndices; k++) {
                indices.push_back(face.mIndices[k]);
            }
        }
        // 一个 mesh 对应一个 material
        if (scene->mMaterials && mesh->mMaterialIndex < scene->mNumMaterials &&
            scene->mMaterials[mesh->mMaterialIndex]) {
            const auto& mat = *(scene->mMaterials[mesh->mMaterialIndex]);
            {
                auto vec = loadTexture(mat, aiTextureType_DIFFUSE, DIFFUSE_TEXTURE_PREFIX);
                textures.insert(textures.end(),
                                std::make_move_iterator(vec.begin()),
                                std::make_move_iterator(vec.end()));
            }
            {
                auto vec = loadTexture(mat, aiTextureType_SPECULAR, SPECULAR_TEXTURE_PREFIX);
                textures.insert(textures.end(),
                                std::make_move_iterator(vec.begin()),
                                std::make_move_iterator(vec.end()));
            }
        }
        meshes.push_back(Mesh(vertices, indices, textures));
        mynode.meshes.push_back(&meshes[meshes.size() - 1]);
    }
    for (int i = 0; i < node->mNumChildren; i++) {
        Node child;
        processNode(scene, node->mChildren[i], child);
        mynode.children.push_back(std::move(child));
    }
}

void Model::load(const std::string &path) {
    Assimp::Importer i;
    const aiScene *scene = i.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
    if (!scene || !scene->HasMeshes()) {
        assert(false);
        return;
    }
    
    directory = path.substr(0, path.find_last_of('/') + 1);
    processNode(scene, scene->mRootNode, rootNode);
}

void Model::draw(const Shader &shader) const {
    for (const auto& mesh: meshes) {
        mesh.draw(shader);
    }
}
