#pragma once

#include <materials/material.h>
#include <textures/texture.h>

class DiffuseLight : public Material
{
private:
    std::shared_ptr<Texture> m_emitter;

public:
    DiffuseLight(std::shared_ptr<Texture> t) : m_emitter(t) {}
    DiffuseLight(Color c) : m_emitter(std::make_shared<SolidColor>(c)) {}

    bool scatter(const Ray &ray, const HitRecord &rec, Color &attenuation, Ray &scattered) const override
    {
        return false;
    }
    
    Color emitted(double u, double v, const Point3 &p) const override
    {
        return m_emitter->value(u, v, p);
    }
};