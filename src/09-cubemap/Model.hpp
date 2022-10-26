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
    void updateTransform(const glm::mat4 &t) {
        transform = t;
        update();
    }
private:
    void setup();
    void makeUsingVertices();
    void sortTextures();
    void update();

    void updateTexture(GLuint Id) { textures[0].ID = Id; }
    const glm::vec3& randomVertex() { return usingVertices[0].position; };
};


class Model {
public:
    Model(const std::string& path, const glm::mat4 &transform, GLenum wrap = GL_REPEAT, Shader shader = 0, bool useMipmap = true): modelMat(transform),
            wrap(wrap), shader(std::move(shader)), useMipmap(useMipmap)
        { load(path); };
    Model(const Model& m): modelMat(m.modelMat), meshes(m.meshes), wrap(m.wrap),
        directory(m.directory), shader(m.shader), rootNode(m.rootNode) {}
    Model(Model&& m): modelMat(std::move(m.modelMat)), shader(m.shader),
        meshes(std::move(m.meshes)), directory(std::move(m.directory)),
        wrap(m.wrap), rootNode(std::move(m.rootNode)) {}

    void draw() const { draw(shader); }
    void draw(const Shader &shader) const;
    Shader& prepareDrawing() { shader.use(); return shader; };
    
    virtual void updateTransform(const glm::mat4 &transform);
    const glm::vec3& randomVertex() { return meshes[0].randomVertex(); };
    void updateTexture(GLuint Id) { meshes[0].updateTexture(Id); }
    /// move
    void setTextures(std::vector<Texture> &&textures);
    void appendTextures(const std::vector<Texture> &textures);
    
public:
    glm::mat4 modelMat;
    Shader shader;
    
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
    std::vector<Texture> loadTexture(const aiMaterial& mat, aiTextureType type, const std::string& namePrefix, bool useMipmap);
    
};


class Bulb: public Model, public PointLight {
public:
    Bulb(const std::string& path, const glm::mat4 &transform,
          GLenum wrap = GL_REPEAT):
        Model(path, transform, wrap), PointLight(transform[3]) {};
    Bulb(const Bulb &bulb): Model(bulb), PointLight(bulb) {}
    Bulb(Bulb &&bulb): Model(bulb), PointLight(bulb) {}

    void updateTransform(const glm::mat4 &transform);
};


class Scene {
    friend class Renderer;
    std::vector<std::shared_ptr<Model>> models;
    std::vector<std::shared_ptr<Light>> lights;
public:
    std::shared_ptr<Model> skybox;
    
public:
    void addModel(const std::string& path, const glm::mat4 &trans, const Shader &shader, GLenum wrap = GL_REPEAT) {
        std::shared_ptr<Model> m(new Model(path, trans, wrap, shader));
        models.push_back(m);
    }

    void addModel(Model* model) {
        models.push_back(std::shared_ptr<Model>(model));
    }
    
    void addModel(std::shared_ptr<Model> model) {
        models.push_back(model);
    }

    void addLight(std::shared_ptr<Light> light) {
        lights.push_back(light);
    }

    Scene() {};
    Scene(const Scene &s): models(s.models), lights(s.lights) {};
    Scene(Scene &&s): models(std::move(s.models)), lights(std::move(s.lights)) {};
    
private:
    Model& modelAt(int index) { return *models[index].get(); };
    Light& lightAt(int index) { return *lights[index].get(); };
    
};

#endif /* Model_hpp */
