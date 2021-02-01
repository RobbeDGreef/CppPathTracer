#pragma once

#include <materials/material.h>
#include <hitable/hitable.h>
#include <textures/texture.h>

class Lambertian : public Material
{
private:
    std::shared_ptr<Texture> m_texture;
public:
    // Will create a default white material
    Lambertian() : m_texture(std::make_shared<SolidColor>()) {}
    
    Lambertian(Color color) : m_texture(std::make_shared<SolidColor>(color)) {}
    Lambertian(std::shared_ptr<Texture> texture) : m_texture(texture) {}
    
    bool scatter(const Ray &ray, const HitRecord &rec, Color &attenuation, Ray &scattered) const override;
};
