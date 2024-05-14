#pragma once

#include <textures/texture.h>
#include <hitables/hitable.h>
#include <materials/material.h>

class Uber : public Material
{
private:
    std::shared_ptr<Texture> m_baseColor;
    double m_roughness = 1;
    bool m_metallic = false;

    std::shared_ptr<Texture> m_emission;
    double m_emission_strength = 0;

public:
    Uber(const std::shared_ptr<Texture> &color, double roughness, bool metalic, const std::shared_ptr<Texture> &emission, double emission_strength) : m_baseColor(color), m_roughness(roughness), m_metallic(metalic), m_emission(emission), m_emission_strength(emission_strength) {}
    bool scatter(const Ray &r, const HitRecord &rec, ScatterRecord &srec) const override;
    bool emitted(double u, double v, const Point3 &p, Color &emission) const override;
    double pdf(const Ray &in, HitRecord rec, const ScatterRecord &srec) const override;
};