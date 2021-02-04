#pragma once

#include <hitables/hitable.h>
#include <bvh/aabb.h>
#include <core.h>

class HitableList : public Hitable
{
private:
    std::vector<HitablePtr> m_objects;

public:
    HitableList() {}
    HitableList(HitablePtr object) { add(object); }

    void add(HitablePtr object) { m_objects.push_back(object); }
    void clear() { m_objects.clear(); }
    std::vector<HitablePtr> objects() const { return m_objects; }

    bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
    bool boundingBox(double t0, double t1, AABB &boundingBox) const override;
};