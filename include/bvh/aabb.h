#pragma once

#include <vec3.h>
#include <ray.h>
#include <sampleable.h>

class AABB
{
private:
    Point3 m_min;
    Point3 m_max;

public:
    AABB() {}
    AABB(const Point3 &min, const Point3 &max) : m_min(min), m_max(max) {}

    void scale(double scale);

    Point3 min() const { return m_min; }
    Point3 max() const { return m_max; }
    double volume() const;
    
    bool hit(const Ray &ray, double t_min, double t_max, double &t) const;
    Point3 randomPointIn() const;

    // static
    static AABB surroundingBox(AABB &b0, AABB &b1);
};