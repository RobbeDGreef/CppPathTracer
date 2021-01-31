#pragma once

#include <vec3.h>
#include <ray.h>

class AABB
{
private:
    Point3 m_min;
    Point3 m_max;

public:
    AABB() {}
    AABB(const Point3 &min, const Point3 &max) : m_min(min), m_max(max) {}

    Point3 min() const { return m_min; }
    Point3 max() const { return m_max; }

    bool hit(const Ray &ray, double t_min, double t_max) const;

    // static
    static AABB surroundingBox(AABB &b0, AABB &b1);
};