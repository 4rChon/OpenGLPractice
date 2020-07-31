#pragma once
#include <glm\ext\matrix_transform.hpp>
#include "../model/model.h"
#include "Transform.h"

class Renderable
{
public:
    Transform transform;
    Model model;
    Renderable(Model& model, Transform transform)
        : model(model), transform(transform) {}

    virtual void Draw(Shader& shader) const
    {
        shader.setMat4("model", transform.GetModel());
        model.Draw(shader);
    }
};