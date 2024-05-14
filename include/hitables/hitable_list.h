#pragma once

#include <hitables/hitable.h>
#include <bvh/aabb.h>
#include <core.h>

class HitableList : public Hitable
{
private:
    std::vector<HitablePtr> m_objects;
    AABB m_box;

public:
    HitableList() {}
    HitableList(HitablePtr object) { add(object); }

    void add(std::shared_ptr<HitableList> object)
    {
        for (const auto &obj : object->objects())
            add(obj);
    }
    void add(HitablePtr object);
    void clear() { m_objects.clear(); }
    std::vector<HitablePtr> objects() const { return m_objects; }

    bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
    bool boundingBox(AABB &bounding_box) const override;
    Point3 randomPointIn() const override;
    double pdf(const Ray &r) const override;
};