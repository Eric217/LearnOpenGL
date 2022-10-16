//
//  Model.hpp
//  06-model
//
//  Created by Eric on 2022/10/10.
//

#ifndef Model_hpp
#define Model_hpp

#include "Shader.h"
#include "Texture.h"

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <glad/glad.h>
#include <assimp/scene.h>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoor;
};

class Mesh {
public:
    
public:
    /// move 构造
    Mesh(std::vector<Vertex> &v, std::vector<GLuint> &i, std::vector<Texture> &t):
        vertices(std::move(v)), indices(std::move(i)), textures(std::move(t))
    {
        setup();
    };
    ~Mesh();
    Mesh(Mesh &&mesh);
    
    void draw(const Shader& shader) const;
    
private:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    
    GLuint vao, vbo, ebo;
    
private:
    void setup();
};


class Model {
private:
    class Node {
        friend class Model;
        std::vector<Mesh *> meshes;
        std::vector<Node> children;
    };
    
    Node rootNode;
    
public:
    Model(const std::string& path) { load(path); };
    
    void draw(const Shader &shader) const;
    
private:
    std::vector<Mesh> meshes;
    std::string directory;
    
private:
    void load(const std::string &path);
    void processNode(const aiScene* scene, aiNode *node, Node &mynode);
    std::vector<Texture> loadTexture(const aiMaterial& mat, aiTextureType type, const std::string& namePrefix);
};
#endif /* Model_hpp */
