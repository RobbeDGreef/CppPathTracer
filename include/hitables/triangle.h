#pragma once

#include <hitables/hitable.h>

class Triangle : public Hitable
{
private:
    Point3 m_points[3];
    std::shared_ptr<Material> m_mat;
    const bool m_doublesided = false;

protected:
    virtual std::shared_ptr<Material> material(double t) const { return m_mat; }

public:
    Triangle(Point3 x, Point3 y, Point3 z, std::shared_ptr<Material> mat) 
             : m_points({x,y,z}), m_mat(mat) {}

    bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
    bool boundingBox(double t0, double t1, AABB &bounding_box) const override;
};