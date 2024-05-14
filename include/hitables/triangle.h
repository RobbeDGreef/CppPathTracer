#pragma once

#include <hitables/hitable.h>

class Triangle : public Hitable
{
private:
    Point3 m_points[3];
    std::shared_ptr<Material> m_mat;
    const bool m_doublesided = false;

protected:
    virtual std::shared_ptr<Material> material() const { return m_mat; }

public:
    Triangle(Point3 x, Point3 y, Point3 z, std::shared_ptr<Material> mat) 
             : m_points{x,y,z}, m_mat(mat) {}

    Point3 x() const { return m_points[0]; }
    Point3 y() const { return m_points[1]; }
    Point3 z() const { return m_points[2]; }
    
    bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
    bool boundingBox(AABB &bounding_box) const override;

    Point3 randomPointIn() const override;
    double pdf(const Ray& r) const override;
};