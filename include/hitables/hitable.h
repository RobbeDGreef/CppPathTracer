#pragma once

#include <ray.h>
#include <vec3.h>
#include <core.h>
#include <bvh/aabb.h>
#include <sampleable.h>

class Material;
class Hitable;
using HitablePtr = Hitable*;


class HitRecord
{
public:
    Point3 p;
    Direction normal;
    double t;
    bool front_face;
    std::shared_ptr<Material> mat;
    double u;
    double v;
    Hitable const * hitable;

    inline void set_face_normal(const Ray &ray, const Direction outward_normal)
    {
        front_face = dot(ray.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class Hitable : public Sampleable
{
public:
    virtual Point3 center() const { return 0; };
    virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const = 0;
    virtual bool boundingBox(AABB &bounding_box) const = 0;

    virtual Point3 randomPointIn() const override;
    virtual double pdf(const Ray& r) const override;
};