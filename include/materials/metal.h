#pragma once

#include <hitables/hitable.h>
#include <materials/material.h>

class Metal : public Material 
{
private:
    Color m_albedo;
    double m_fuzzyness;
    
public:
    Metal(const Color &color, double fuzzyness=0.0) : m_albedo(color), m_fuzzyness(fuzzyness) {}
    bool scatter(const Ray &ray, const HitRecord &rec, Color &attenuation, Ray &scattered) const override;
};