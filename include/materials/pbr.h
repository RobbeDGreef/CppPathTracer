#pragma once

#include <textures/texture.h>
#include <hitables/hitable.h>
#include <materials/material.h>

#if 0

class Pbr : public Material
{
private:
    std::shared_ptr<Texture> m_baseColor;
    double m_roughness = 1;
    double m_metallic = 0;

    std::shared_ptr<Texture> m_emission;
    double m_emission_strength = 0;

public:
    Pbr(const std::shared_ptr<Texture> &color, double roughness, double metalic, const std::shared_ptr<Texture> &emission, double emission_strength) : m_baseColor(color), m_roughness(roughness), m_metallic(metalic), m_emission(emission), m_emission_strength(emission_strength) {}
    Color color(const Ray &r, const HitRecord &rec, const Ray &light_ray) const override;
    bool scatter(const Ray &r, const HitRecord &rec, Ray &scattered, double &pdf) const override;
    bool emitted(double u, double v, const Point3 &p, Color &emission) const override;
    double pdf(const Ray &in, const Ray &out, const HitRecord &rec) const override;
};

#endif