#pragma once

#include <hitables/hitable.h>
#include <materials/material.h>
#include <bvh/aabb.h>

class Sphere : public Hitable
{
protected:
    Point3 m_center;
    double m_radius = 0;
    std::shared_ptr<Material> m_mat;

private:
    static void getUV(const Point3 &p, double &u, double &v);

protected:
    virtual Point3 center() const { return m_center; }
    virtual double radius() const { return m_radius; }
    virtual std::shared_ptr<Material> material() const { return m_mat; }

public:
    Sphere(Point3 center, double radius, std::shared_ptr<Material> mat) 
        : m_center(center), m_radius(radius), m_mat(mat) {}
    Sphere() {} 
    bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
    bool boundingBox(AABB &bounding_box) const override;
};