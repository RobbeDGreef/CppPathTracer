#pragma once

#include <materials/material.h>
#include <hitable/hitable.h>

class Lambertian : public Material
{
private:
    Color m_albedo;
public:
    Lambertian() {}
    Lambertian(Color color) : m_albedo(color) {}
    bool scatter(const Ray &ray, const HitRecord &rec, Color &attenuation, Ray &scattered) const override;
};
