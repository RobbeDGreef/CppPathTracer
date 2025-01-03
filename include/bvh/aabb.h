#pragma once

#include <vec3.h>
#include <ray.h>
#include <sampleable.h>

class Hitable;
using HitablePtr = Hitable*;

class AABB
{
private:
    Point3 m_min;
    Point3 m_max;

public:
    AABB() {}
    AABB(const Point3 &min, const Point3 &max) : m_min(min), m_max(max) {}
    AABB(const std::vector<HitablePtr>& objects);

    void scale(double scale);

    Point3 minPoint() const { return m_min; }
    Point3 maxPoint() const { return m_max; }
    double volume() const;
    double surfaceArea() const;

    bool hit(const Ray &ray, double t_min, double t_max) const;
    Point3 randomPointIn() const;

    // static
    static AABB surroundingBox(AABB &b0, AABB &b1);
};