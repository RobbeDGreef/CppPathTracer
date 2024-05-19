#include <bvh/aabb.h>
#include <vec3.h>
#include <random.h>
#include <hitables/hitable.h>

bool AABB::hit(const Ray &ray, double t_min, double t_max, double &t) const
{
    t = t_min;
    for (int i = 0; i < 3; i++)
    {
        double inverted_d = 1.0 / ray.direction()[i];
        double t0 = (minPoint()[i] - ray.origin()[i]) * inverted_d;
        double t1 = (maxPoint()[i] - ray.origin()[i]) * inverted_d;

        if (inverted_d < 0.0)
            std::swap(t0, t1);

        t_min = t0 > t_min ? t0 : t_min;
        t_max = t1 < t_max ? t1 : t_max;

        if (t_max <= t_min)
            return false;

        // TODO: i am not sure if this is actually right.
        t = t_min > t ? t_min : t;
    }

    return true;
}

AABB AABB::surroundingBox(AABB &b0, AABB &b1)
{
    return AABB(minValues(b0.minPoint(), b1.minPoint()), maxValues(b0.maxPoint(), b1.maxPoint()));
}

Point3 AABB::randomPointIn() const
{
    return randomGen.getPoint3() * (m_max - m_min) + m_min;
}

double AABB::volume() const
{
    auto lengths = m_max - m_min;
    return lengths[0] * lengths[1] * lengths[2];
}

double AABB::surfaceArea() const
{
    auto lengths = maxPoint() - minPoint();
    return (lengths[0] * lengths[1] * 2 +
            lengths[2] * lengths[1] * 2 +
            lengths[2] * lengths[0] * 2);
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
