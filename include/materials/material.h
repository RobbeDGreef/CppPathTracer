#pragma once

#include <ray.h>
#include <vec3.h>
#include <hitable/hitable.h>

class Material
{
public: 
    virtual bool scatter(const Ray &r, const HitRecord &rec, Color &attenuation, Ray &scattered) const = 0;
};