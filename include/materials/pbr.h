#pragma once

#include <hitables/hitable.h>
#include <materials/material.h>

class Pbr : public Material
{
private:
    Color m_baseColor;
    double m_roughness = 1;
    double m_metalic = 1;

    Color m_emission = Color(0, 0, 0);
    double m_emission_strength = 0;

public:
    Pbr(const Color &color, double roughness, double metalic, const Color &emission, double emission_strength) : m_baseColor(color), m_roughness(roughness), m_metalic(metalic), m_emission(emission), m_emission_strength(emission_strength) {}
    bool scatter(const Ray &r, const HitRecord &rec, Color &attenuation, Ray &scattered) const override;
    bool emitted(double u, double v, const Point3 &p, Color &emission) const override;
};