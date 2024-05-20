#pragma once

#include <hitables/hitable.h>
#include <hitables/hitable_list.h>
#include <config.h>

class BvhNode : public Hitable
{
private:
    HitablePtr m_left = nullptr;
    HitablePtr m_right = nullptr;
    AABB m_box;

public:
    BvhNode() {}
    BvhNode(const HitableList &list)
        : BvhNode(list.objects(), 0, list.objects().size()) {}

    BvhNode(const std::vector<HitablePtr> &objects, int start, int end);

    bool is_uninitialized() const
    {
        return m_left.get() == nullptr || m_right.get() == nullptr;
    }
    bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
    bool boundingBox(AABB &bounding_box) const override;
};