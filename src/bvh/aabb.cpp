#include <bvh/aabb.h>
#include <vec3.h>
#include <random.h>
#include <hitables/hitable.h>
#include <config.h>

bool AABB::hit(const Ray &ray, double t_min, double t_max) const
{

    // Branchless implementation of unoptimized slab method AABB testing
    // see https://tavianator.com/2011/ray_box.html.

    Point3 min = minPoint();
    Point3 max = maxPoint();

    Direction inverted_d = ray.inverted_direction();

#if AABB_HIT_IMPLEMENTATION == AABB_HIT_BRANCHLESS_VECTOR

    Vec3d t0s = (min - ray.origin()) * inverted_d;
    Vec3d t1s = (max - ray.origin()) * inverted_d;

    Vec3d mins = minValues(t0s, t1s);
    Vec3d maxs = maxValues(t0s, t1s);

    double final_tmin = maxVal(mins);
    double final_tmax = minVal(maxs);

    return std::max(t_min, final_tmin) <= final_tmax && final_tmin < t_max;
#endif

#if AABB_HIT_IMPLEMENTATION == AABB_HIT_BRANCHLESS

    double tx1 = (min.x() - ray.origin().x()) * inverted_d.x();
    double tx2 = (max.x() - ray.origin().x()) * inverted_d.x();

    double cur_min = std::min(tx1, tx2);
    double cur_max = std::max(tx1, tx2);

    double ty1 = (min.y() - ray.origin().y()) * inverted_d.y();
    double ty2 = (max.y() - ray.origin().y()) * inverted_d.y();

    cur_min = std::max(cur_min, std::min(ty1, ty2));
    cur_max = std::min(cur_max, std::max(ty1, ty2));

    double tz1 = (min.z() - ray.origin().z()) * inverted_d.z();
    double tz2 = (max.z() - ray.origin().z()) * inverted_d.z();

    cur_min = std::max(cur_min, std::min(tz1, tz2));
    cur_max = std::min(cur_max, std::max(tz1, tz2));

    return std::max(t_min, cur_min) <= cur_max && cur_min < t_max;

#endif

#if AABB_HIT_IMPLEMENTATION == AABB_HIT_NAIVE

    for (int i = 0; i < 3; i++)
    {
        double t0 = (min[i] - ray.origin()[i]) * inverted_d[i];
        double t1 = (max[i] - ray.origin()[i]) * inverted_d[i];

        if (inverted_d[i] < 0.0)
            std::swap(t0, t1);

        t_min = t0 > t_min ? t0 : t_min;
        t_max = t1 < t_max ? t1 : t_max;

        if (t_max <= t_min)
            return false;
    }

    return true;
#endif
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
    Vec3d diff = m_max - m_min;
    Vec3d scaled_diff = diff * scale;
    Vec3d difference = diff - scaled_diff;

    // Scale along the center of this box

    m_min += difference / 2;
    m_max = m_min + scaled_diff;
}
