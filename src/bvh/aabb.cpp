#include <bvh/aabb.h>
#include <vec3.h>

bool AABB::hit(const Ray &ray, double t_min, double t_max) const
{
    for (int i = 0; i < 3; i++)
    {
        double inverted_d = 1.0 / ray.direction()[i];
        double t0 = (min()[i] - ray.origin()[i]) * inverted_d;
        double t1 = (max()[i] - ray.origin()[i]) * inverted_d;

        if (inverted_d < 0.0)
            std::swap(t0, t1);

        t_min = t0 > t_min ? t0 : t_min;
        t_max = t1 < t_max ? t1 : t_max;

        if (t_max <= t_min)
            return false;
    }

    return true;
}

AABB AABB::surroundingBox(AABB &b0, AABB &b1)
{
    return AABB(minValues(b0.min(), b1.min()), maxValues(b0.max(), b1.max()));
}
void AABB::scale(double scale)
{
    Vec3<double> diff = m_max - m_min;
    Vec3<double> scaled_diff = diff * scale;
    Vec3<double> difference = diff - scaled_diff;

    // Scale along the center of this box

    m_min += difference / 2;
    m_max = m_min + scaled_diff;
}
