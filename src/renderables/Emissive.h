#pragma once
#include "Renderable.h"
#include "../lights/light.h"
#include "../model/model.h"
class Emissive : public Renderable
{
public:
    Light* light;
    Emissive(Model& model, Light* light, Transform transform)
        :  Renderable(model, transform), light(light) {
    }

    virtual void Draw(Shader& shader) const override
    {
        shader.setMat4("model", transform.GetModel());
        shader.setVec3("color", light->color);
        model.Draw(shader);
    }
};