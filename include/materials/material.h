#pragma once

#include <ray.h>
#include <vec3.h>
#include <hitables/hitable.h>

class Material
{
public: 
    virtual bool scatter(const Ray &r, const HitRecord &rec, Color &attenuation, Ray &scattered) const = 0;
    virtual Color emitted(double u, double v, const Point3 &p) const { return Color(0,0,0); }
};