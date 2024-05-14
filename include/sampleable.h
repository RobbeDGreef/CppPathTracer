#pragma once

#include <vec3.h>
#include <ray.h>

class Sampleable 
{
public:
    virtual Point3 randomPointIn() const = 0;
    virtual double pdf(const Ray& r) const = 0;
};