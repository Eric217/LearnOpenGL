//
//  Scene.cpp
//  12-shadow
//
//  Created by Eric on 2022/11/12.
//

#include "Scene.hpp"

Scene::ModelArray Scene::allModels() const {
    ModelArray _models = models;
    _models.push_back(skybox);
    return _models;
}
 
