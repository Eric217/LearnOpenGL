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
#include "Light.h"
#include "Framebuffer.hpp"

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <glad/glad.h>
#include <assimp/scene.h>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoor;
    
    void assignVec3(int slot, const glm::vec4 &pos) {
        auto position = (glm::vec3 *)this + slot;
        
        position->x = pos.x;
        position->y = pos.y;
        position->z = pos.z;
    }
};

class Mesh {
    friend class Model;
    friend class Renderer;

    std::vector<Vertex> vertices;
    std::vector<Vertex> usingVertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    
    GLuint vao, vbo, ebo;
    GLenum wrap;
public:
    glm::mat4 transform;
    
public:
    /// move 构造
    Mesh(std::vector<Vertex> &v, std::vector<GLuint> &i, std::vector<Texture> &t, const glm::mat4 &transform,
         GLenum wrap): wrap(wrap),
        vertices(std::move(v)), indices(std::move(i)),
        textures(std::move(t)), transform(transform)
    {
        setup();
    };
    ~Mesh();
    Mesh(const Mesh &mesh);
    Mesh(Mesh &&mesh);
    
    void draw(const Shader& shader) const;
    void draw(const Shader& shader, const std::vector<Texture> &textures) const;
    
    void updateTransform(const glm::mat4 &t) {
        transform = t;
        update();
    }
private:
    void setup();
    void makeUsingVertices();
    void sortTextures();
    void update();

    void applyTextures(const Shader &shader) const;
    void applyTextures(const Shader &shader, const std::vector<Texture> &textures) const;
    
    void updateTexture(GLuint Id) { textures[0].ID = Id; }
};


class Model {
    friend class Renderer;
public:
    Model(const std::string& path, Shader shader = 0,
          const glm::mat4 &transform = mat4(1), GLenum wrap = GL_REPEAT,
          bool useMipmap = true):
    modelMat(transform), wrap(wrap), shader(std::move(shader)),
    useMipmap(useMipmap) { load(path); };
    
    Model(const Model& m): modelMat(m.modelMat), meshes(m.meshes), wrap(m.wrap),
        directory(m.directory), shader(m.shader), rootNode(m.rootNode), useMipmap(m.useMipmap) {}
    Model(Model&& m): modelMat(std::move(m.modelMat)), shader(m.shader),
        meshes(std::move(m.meshes)), directory(std::move(m.directory)),
        wrap(m.wrap), rootNode(std::move(m.rootNode)), useMipmap(m.useMipmap) {}

    void draw() const { draw(shader); }
    void draw(const Shader &shader) const;
    void draw(const std::vector<Texture> &textures) const;
    void draw(const Shader &shader, const std::vector<Texture> &textures) const;
    void draw(const Shader &shader, const std::vector<Texture> &textures, const Framebuffer &buffer, GLbitfield clearMask) const;

    Shader& prepareDrawing() { shader.use(); return shader; };
    
    virtual void updateTransform(const glm::mat4 &transform);
    /// danger!
    void updateTexture(GLuint Id) { meshes[0].updateTexture(Id); }
    /// move
    void setTextures(std::vector<Texture> &&textures);
    void appendTextures(const std::vector<Texture> &textures);
    void appendUniqueTextures(const std::vector<Texture> &textures);

    void bindUniformBlock(const std::string &name, int slot) const;
    
public:
    glm::mat4 modelMat;
    Shader shader;
//    bool cull
    
private:
    std::vector<Mesh> meshes;
    std::string directory;
    GLenum wrap;
    bool useMipmap;

private:
    class Node {
        friend class Model;
        std::vector<Mesh *> meshes;
        std::vector<Node> children;
    };
    Node rootNode;
    
protected:
    void load(const std::string &path);
    void processNode(const aiScene* scene, aiNode *node, Node &mynode);
    std::vector<Texture> loadTexture(const aiMaterial& mat, aiTextureType type, TextureType tType, bool useMipmap);
    
};


class Bulb: public Model, public PointLight {
public:
    Bulb(const std::string& path, const glm::mat4 &transform,
          GLenum wrap = GL_REPEAT):
        Model(path, 0, transform, wrap), PointLight(transform[3]) {};
    Bulb(const Bulb &bulb): Model(bulb), PointLight(bulb) {}
    Bulb(Bulb &&bulb): Model(bulb), PointLight(bulb) {}

    void updateTransform(const glm::mat4 &transform);
};

class Sun: public Model, public DirLight {
public:
public:
    Sun(const std::string& path, Shader shader, const glm::mat4 &transform,
          GLenum wrap = GL_REPEAT):
        Model(path, shader, transform, wrap) {};
private:
    Sun(const Sun &sun): Model(sun) {};
    Sun(Sun &&sun): Model(sun) {};
};

class DeferredShadingData {
public:
    Model quad;
    Shader mrtShader;
    std::vector<std::shared_ptr<Model>> volumes;
    Shader volumeShader;
public:
    DeferredShadingData(const Model &quad, const Shader &shader):
        quad(quad), mrtShader(shader) {}
};

#endif /* Model_hpp */
