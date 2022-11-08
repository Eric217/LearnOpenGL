//
//  Model.cpp
//  06-model
//
//  Created by Eric on 2022/10/10.
//

#include "Model.hpp"

void Mesh::setup() {
    makeUsingVertices();
    sortTextures();
    
    GLuint buffers[2];
    glGenBuffers(2, buffers);
    vbo = buffers[0];
    ebo = buffers[1];
    glGenVertexArrays(1, &vao);
    
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData (GL_ARRAY_BUFFER, usingVertices.size() * sizeof(Vertex),
                  &usingVertices[0], GL_STATIC_DRAW);
    
    glVertexAttribPointer (0, 3, GL_FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, position));
    glVertexAttribPointer (1, 3, GL_FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    glVertexAttribPointer (2, 2, GL_FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, texCoor));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
}

void Mesh::makeUsingVertices() {
    bool usePush = usingVertices.size() != vertices.size();
    auto normalMat = glm::transpose(glm::inverse(transform));
    
    if (usePush) {
        usingVertices.clear();
        if (normalMat == mat4(1)) {
            usingVertices = vertices;
            return;
        }
    }
     
    for (int i = 0; i < vertices.size(); i++) {
        if (usePush) {
            auto v = vertices[i];
            v.assignVec3(0, transform * glm::vec4(v.position, 1));
            v.assignVec3(1, glm::normalize(normalMat * glm::vec4(v.normal, 0)));
            usingVertices.push_back(std::move(v));
        } else {
            usingVertices[i].assignVec3(0, transform * glm::vec4(vertices[i].position, 1));
            usingVertices[i].assignVec3(1, glm::normalize(normalMat * glm::vec4(vertices[i].normal, 0)));
        }
    }
}

void Mesh::update() {
    makeUsingVertices();
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData (GL_ARRAY_BUFFER, usingVertices.size() * sizeof(Vertex),
                  &usingVertices[0], GL_DYNAMIC_DRAW);
}

Mesh::Mesh(Mesh &&mesh) {
    vao = mesh.vao;
    vbo = mesh.vbo;
    ebo = mesh.ebo;
    wrap = mesh.wrap;
    transform = std::move(mesh.transform);
    vertices = std::move(mesh.vertices);
    indices = std::move(mesh.indices);
    textures = std::move(mesh.textures);
    usingVertices = std::move(mesh.usingVertices);
}

Mesh::Mesh(const Mesh &mesh) {
    vao = mesh.vao;
    vbo = mesh.vbo;
    ebo = mesh.ebo;
    wrap = mesh.wrap;
    transform = mesh.transform;
    vertices = mesh.vertices;
    indices = mesh.indices;
    textures = mesh.textures;
    usingVertices = mesh.usingVertices;
}

Mesh::~Mesh() {
}

static std::string samplerNamePrefix = "material.";
static std::string useSamplerNamePrefix = "material.use_";

void Mesh::applyTextures(const Shader &shader) const {
    // 每个纹理绑到一个槽位上，按照类型绑到不同的 uniform sampler 上
    auto diffNr = 0, specNr = 0, cubeNr = 0, cube2dNr = 0,
    dirLightNr = 0;
    
    for (auto i = 0; i < textures.size(); i++) {
        textures[i].use(i, wrap);
        if (textures[i].type == DIFFUSE_TEXTURE) {
            auto s = textures[i].type + std::to_string(diffNr++);
            shader.setInt(samplerNamePrefix + s, i);
            shader.setBool(useSamplerNamePrefix + s, textures[i].shouldUse);
        } else if (textures[i].type == SPECULAR_TEXTURE) {
            auto s = textures[i].type + std::to_string(specNr++);
            shader.setInt(samplerNamePrefix + s, i);
            shader.setBool(useSamplerNamePrefix + s, textures[i].shouldUse);
        } else if (textures[i].type == CUBEMAP_TEXTURE) {
            auto s = textures[i].type + std::to_string(cubeNr++);
            shader.setInt(samplerNamePrefix + s, i);
            shader.setBool(useSamplerNamePrefix + s, textures[i].shouldUse);
        } else if (textures[i].type == CUBEMAP2D_TEXTURE) {
            auto s = textures[i].type + std::to_string(cube2dNr++);
            shader.setInt(samplerNamePrefix + s, i);
            shader.setBool(useSamplerNamePrefix + s, textures[i].shouldUse);
        } else if (textures[i].type == DIR_LIGHT_TEXTURE) {
            auto s = textures[i].type + std::to_string(dirLightNr++);
            shader.setInt("lights." + s, i);
            shader.setBool("lights.use_" + s, textures[i].shouldUse);
        } else {
            assert(false); // 未处理!
        }
    }
}

void Mesh::draw(const Shader &shader) const {
    glBindVertexArray(vao);
    applyTextures(shader);
    glDrawElements (GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

#include <map>
#include <iostream>

static std::map<std::string, int> texture_order = {
    {DIFFUSE_TEXTURE, 0},
    {SPECULAR_TEXTURE, 1},
    {CUBEMAP2D_TEXTURE, 2},
    {CUBEMAP_TEXTURE, 3},
    {DIR_LIGHT_TEXTURE, 4},
};

void Mesh::sortTextures() {
    if (textures.size() <= 1) {
        return;
    }
    std::sort(textures.begin(), textures.end(), [](Texture &left, Texture& right) {
        return texture_order[left.type] < texture_order[right.type];
    });
        
    std::string orders("texture orders:");
    for (Texture &t : textures) {
        orders.append(std::to_string(texture_order[t.type]) + ", ");
    }
    std::cout << orders << std::endl;
}

// MARK: - Model
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

std::vector<Texture> Model::loadTexture(const aiMaterial& mat, aiTextureType type, const std::string& namePrefix, bool useMipmap) {
    std::vector<Texture> textures;
    
    auto dCount = mat.GetTextureCount(type);

    for (int j = 0; j < dCount; j++) {
        aiString p;
        mat.GetTexture(type, j, &p);
        if (!p.length) {
            continue;
        }
        auto path = directory + p.C_Str();
        auto t = Texture::load(path, namePrefix, useMipmap);
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
            v.position = glm::vec4(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z, 1);
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
                auto vec = loadTexture(mat, aiTextureType_DIFFUSE, DIFFUSE_TEXTURE, useMipmap);
                textures.insert(textures.end(),
                                std::make_move_iterator(vec.begin()),
                                std::make_move_iterator(vec.end()));
            }
            {
                auto vec = loadTexture(mat, aiTextureType_SPECULAR, SPECULAR_TEXTURE, useMipmap);
                textures.insert(textures.end(),
                                std::make_move_iterator(vec.begin()),
                                std::make_move_iterator(vec.end()));
            }
        }
        meshes.push_back(Mesh(vertices, indices, textures, modelMat, wrap));
        mynode.meshes.push_back(&meshes[meshes.size() - 1]);
    }
    for (int i = 0; i < node->mNumChildren; i++) {
        Node child;
        processNode(scene, node->mChildren[i], child);
        mynode.children.push_back(std::move(child));
    }
}

void Model::updateTransform(const glm::mat4 &transform) {
    modelMat = transform;
    for (auto& mesh: meshes) {
        mesh.updateTransform(transform);
    }
}

void Model::setTextures(std::vector<Texture> &&textures) {
    meshes[0].textures = textures;
    meshes[0].sortTextures();
}

void Model:: appendTextures(const std::vector<Texture> &textures) {
    meshes[0].textures.insert(meshes[0].textures.end(), textures.begin(), textures.end());
    meshes[0].sortTextures();
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
    shader.use();
    for (const auto& mesh: meshes) {
        mesh.draw(shader);
    }
}

void Model::bindUniformBlock(const std::string &name, int slot) const {
    shader.bindUniformBlock(name, slot);
}

// MARK: - Bulb

void Bulb::updateTransform(const glm::mat4 &transform) {
    Model::updateTransform(transform);
    position = transform[3];
}

Scene::ModelArray Scene::allModels() const {
    ModelArray _models = models;
    _models.push_back(skybox);
    return _models;
}

//const Light& Scene::pointLight() const {
//    for (int i = 0; i<lights.size(); i++) {
//        auto p = dynamic_cast<PointLight*>(lights[i].get());
//        assert(p);
//        return *p;
//    }
//}
