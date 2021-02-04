#include <hitable/hitable_list.h>

bool HitableList::hit(const Ray &ray, double t_min, double t_max, HitRecord &rec) const
{
    HitRecord tmp;
    bool hit_anything = false;
    double closest_so_far = t_max;

    for (const auto &object : m_objects)
    {
        if (object->hit(ray, t_min, closest_so_far, tmp))
        {
            hit_anything = true;
            closest_so_far = tmp.t;
            rec = tmp;
        }
    }

    return hit_anything;
}

bool HitableList::boundingBox(double t0, double t1, AABB &boundingBox) const
{
    if (m_objects.empty()) return false;

    AABB ret;
    bool first_box = true;

    for (const auto &object : m_objects)
    {
        // If the bounding box of one of the children cannot be created
        // then this bounding box is also invalid and unable to be created
        if (!object->boundingBox(t0,t1, ret)) return false;
        
        if (first_box)
        {
            boundingBox = ret;
            first_box = false;
        }
        else
            boundingBox = AABB::surroundingBox(boundingBox, ret);
    }

    return true;
}