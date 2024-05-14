#include <hitables/hitable_list.h>
#include <random.h>

bool HitableList::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const
{
    HitRecord rec_tmp;
    double closest_hit = t_max;
    bool has_hit = false;

    for (const auto &hitable : m_objects)
    {
        // Try to find a closer hit by setting tmax to closest hit found yet
        if (hitable->hit(r, t_min, closest_hit, rec_tmp))
        {
            closest_hit = rec_tmp.t;
            rec = rec_tmp;
            has_hit = true;
        }
    }

    return has_hit;
}

bool HitableList::boundingBox(AABB &bounding_box) const
{
    bounding_box = m_box;
    return true;
}

Point3 HitableList::randomPointIn() const
{
    return m_objects.at(randomGen.getInt() % m_objects.size())->randomPointIn();
}

double HitableList::pdf(const Ray &r) const
{
    double w = 1.0 / m_objects.size();
    double sum = 0.00001;

    for (const auto &obj : m_objects)
    {
        sum += w * obj->pdf(r);
    }

    return sum;
}

void HitableList::add(HitablePtr object)
{
    AABB object_box;
    object->boundingBox(object_box);
    if (m_objects.size() == 0)
    {
        m_box = object_box;
    }
    else
    {
        m_box = AABB::surroundingBox(m_box, object_box);
    }

    m_objects.push_back(object);
}