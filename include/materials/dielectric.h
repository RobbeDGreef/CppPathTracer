#pragma once

#if 0


#include <materials/material.h>

class Dielectric : public Material
{
private:
    double m_refraction_index;
    
public:
    Dielectric(double refraction_index) : m_refraction_index(refraction_index) {}
    bool scatter(const Ray &ray, const HitRecord &rec, Color &attenuation, Ray &scattered) const override;
};

#endif