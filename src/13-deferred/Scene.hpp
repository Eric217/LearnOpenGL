//
//  Scene.hpp
//  12-shadow
//
//  Created by Eric on 2022/11/12.
//

#ifndef Scene_hpp
#define Scene_hpp

#include "Model.hpp"


class Scene {
    friend class Renderer;
    std::vector<std::shared_ptr<Model>> models;
    std::vector<std::shared_ptr<Light>> lights;
    std::shared_ptr<Model> skybox;
    std::shared_ptr<Model> hdrQuad;
    std::shared_ptr<DeferredShadingData> deferredData;

    Shader gaussionFilter;

public:
    void addModel(const std::string& path, const glm::mat4 &trans, const Shader &shader, GLenum wrap = GL_REPEAT) {
        std::shared_ptr<Model> m(new Model(path, shader, trans, wrap));
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
    
    void addLight(Light *light) {
        lights.push_back(std::shared_ptr<Light>(light));
    }

    Scene() {};
    Scene(const Scene &s): models(s.models), lights(s.lights) {
        assert(false);
    };
    Scene(Scene &&s): models(std::move(s.models)), lights(std::move(s.lights)) {
        assert(false);
    };
    
    typedef std::shared_ptr<Model> ModelPtr;
    typedef std::vector<std::shared_ptr<Model>> ModelArray;
    
    ModelArray allModels() const;
    Model& getHdrQuad() const { return *hdrQuad; }
    Model& getDeferredQuad() const { return deferredData.get()->quad; }
    Shader& getDeferredMRTShader() const { return deferredData.get()->mrtShader; }
    Shader& getPointLightShader() const { return deferredData.get()->volumeShader; }
    std::vector<std::shared_ptr<Model>>& getLightVolumes() const { return deferredData.get()->volumes; }

    void setHdrQuad(Model *model) { hdrQuad.reset(model); }
    void setBloomShader(const Shader &shader) { gaussionFilter = shader; }
    void setDeferredData(DeferredShadingData *data) { deferredData.reset(data); }

    const std::vector<std::shared_ptr<Light>>& getLights() const { return lights; }
    template<class T>
    std::vector<T*> getLights() const {
        std::vector<T*> vec;
        for (int i = 0; i < lights.size(); i++) {
            auto p = dynamic_cast<T*>(lights[i].get());
            if (p)
                vec.push_back(p);
        }
        return vec;
    }
    
private:
    Model& modelAt(int index) const { return *models[index].get(); };
    Light& lightAt(int index) const { return *lights[index].get(); };
    
};
#endif /* Scene_hpp */
