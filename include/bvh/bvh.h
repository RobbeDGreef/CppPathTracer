#pragma once

#include <hitable/hitable.h>
#include <hitable/hitable_list.h>

class BvhNode : public Hitable
{
private:
    HitablePtr m_left;
    HitablePtr m_right;
    AABB m_box;

public:
    BvhNode();
    BvhNode(const HitableList &list, double t0, double t1) 
           : BvhNode(list.objects(), 0, list.objects().size(), t0 ,t1) {}
    
    BvhNode(const std::vector<HitablePtr> &objects, int start, int end, double t0, double t1);

    bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
    bool boundingBox(double t0, double t1, AABB &bounding_box) const override;

    //std::shared_ptr<Hitable> left() const { return m_left; }
    //std::shared_ptr<Hitable> right() const { return m_right; }
};