#include <bvh/bvh.h>
#include <random.h>

bool BvhNode::hit(const Ray &ray, double t_min, double t_max, HitRecord &rec) const
{
    if (!m_box.hit(ray, t_min, t_max))
        return false;

    bool hit_left = m_left->hit(ray, t_min, t_max, rec);
    bool hit_right = m_right->hit(ray, t_min, hit_left ? rec.t : t_max, rec);

    return hit_left || hit_right;
}

static bool hitableCompare(HitablePtr &a, HitablePtr &b, int axis)
{
    AABB a_box;
    AABB b_box;
    
    if (!a->boundingBox(0, 0, a_box) || !b->boundingBox(0,0,b_box))
        ERROR("No bounding box could be constructed in the BVH");
    
    return a_box.min()[axis] < a_box.min()[axis];
}

BvhNode::BvhNode(const std::vector<HitablePtr> &_objects, int start, int end, double t0, double t1)
{
    auto objects = _objects;

    int axis = randomInt(0,2);
    
    auto comparator = [axis](HitablePtr &a, HitablePtr &b) { return hitableCompare(a,b,axis); };

    int length = end - start;

    switch (length)
    {
    case 1:
        m_left = m_right = objects[start];
        break;
    case 2:
        m_left = objects[start+1];
        m_right = objects[start];
        if (comparator(objects[start], objects[start+1]))
            std::swap(m_left, m_right);
        
        break;
    default:
        std::sort(objects.begin() + start, objects.begin() + end, comparator);

        auto mid = start + length / 2;
        m_left = std::make_shared<BvhNode>(objects, start, mid, t0, t1);
        m_right = std::make_shared<BvhNode>(objects, mid, end, t0, t1);
    }

    AABB box_left;
    AABB box_right;
    
    if (!m_left->boundingBox(t0, t1, box_left) || !m_right->boundingBox(t0,t1, box_right))
        ERROR("No bounding box could be constructed in the BVH");
    
    m_box = AABB::surroundingBox(box_left, box_right);
}

bool BvhNode::boundingBox(double t0, double t1, AABB &bounding_box) const
{
    bounding_box = m_box;
    return true;
}