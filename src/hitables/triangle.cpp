#include <hitables/triangle.h>
#include <random.h>
#include <core.h>
#include <config.h>

#if TRIANGLE_INTERSECTION_ALGO == TRIANGLE_INTERSECTION_CRAMMER

inline double det3x3(Direction &a, Direction &b, Direction &c)
{
    const double a00 = a[0], a01 = a[1], a02 = a[2];
    const double a10 = b[0], a11 = b[1], a12 = b[2];
    const double a20 = c[0], a21 = c[1], a22 = c[2];
    return a00 * (a22 * a11 - a12 * a21) +
           a01 * (-a22 * a10 + a12 * a20) +
           a02 * (a21 * a10 - a11 * a20);
}

bool Triangle::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const
{
    const double epsilon = 0.0000001;

    // This is essentially branchless Möller Trumbore.
    // See https://x.com/lisyarus/status/1786327676120117683/photo/2
    Point3 hit;

    Direction dir = r.direction();
    Direction edge1 = m_points[0] - m_points[1];
    Direction edge2 = m_points[0] - m_points[2];
    Direction rhs = m_points[0] - r.origin();

    double d = det3x3(dir, edge1, edge2);
    double t = det3x3(rhs, edge1, edge2) / d;
    double u = det3x3(dir, rhs, edge2) / d;
    double v = det3x3(dir, edge1, rhs) / d;

    bool backfaced = dot(edge1, cross(dir, edge2)) < epsilon;

    rec.p = r.at(t);
    rec.t = t;
    rec.u = u;
    rec.v = v;
    rec.mat = material();
    rec.set_face_normal(r, normalize(cross(edge1, edge2)));

    return (!backfaced || m_doublesided) && t >= 0.0 && u >= 0.0 && v >= 0.0 && u + v <= 1.0;
}
#endif

#if TRIANGLE_INTERSECTION_ALGO == TRIANGLE_INTERSECTION_MOLLER_TRUMBORE

bool Triangle::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const
{
    // Möller-Trumbore intersection algorithm

    const double epsilon = 0.0000001;
    // The UV coordinates
    double u, v;

    Direction edge1 = m_points[1] - m_points[0];
    Direction edge2 = m_points[2] - m_points[0];
    Point3 pvec = cross(r.direction(), edge2);
    double d = dot(edge1, pvec);

    // If the determinant is negative, the triangle is backfacing
    if (d < epsilon && !m_doublesided)
        return false;

    // The ray and the triangle are parallel
    if (fabs(d) < epsilon)
        return false;

    float inverted_d = 1 / d;
    Direction tvec = r.origin() - m_points[0];
    u = dot(tvec, pvec) * inverted_d;

    // U cannot be greater than 1 or smaller than 0, since, well, that is
    // kinda the definition of the UV coordinates.
    if (u < 0 || u > 1)
        return false;

    Point3 qvec = cross(tvec, edge1);
    v = dot(r.direction(), qvec) * inverted_d;

    if (v < 0 || u + v > 1)
        return false;

    // Now we are sure the ray intersects the triangle
    double t = dot(edge2, qvec) * inverted_d;

    if (t > t_max || t < t_min)
        return false;

    rec.p = r.at(t);
    rec.t = t;
    rec.u = u;
    rec.v = v;
    rec.mat = material();
    rec.set_face_normal(r, normalize(cross(edge1, edge2)));

    return true;
}

#endif

bool Triangle::boundingBox(AABB &bounding_box) const
{
    // Is there a better way?
    double z_min = fmin(fmin(m_points[0].z(), m_points[1].z()), m_points[2].z());
    double z_max = fmax(fmax(m_points[0].z(), m_points[1].z()), m_points[2].z());
    double x_min = fmin(fmin(m_points[0].x(), m_points[1].x()), m_points[2].x());
    double x_max = fmax(fmax(m_points[0].x(), m_points[1].x()), m_points[2].x());
    double y_min = fmin(fmin(m_points[0].y(), m_points[1].y()), m_points[2].y());
    double y_max = fmax(fmax(m_points[0].y(), m_points[1].y()), m_points[2].y());

    const double e = 0.0001;
    // Sometimes triangles are x y or z plane aligned and we get an infinitely thin AABB
    // which breaks the BVH. To solve this we simply make sure we add a small amount of padding.
    bounding_box = AABB(Point3(x_min - e, y_min - e, z_min - e), Point3(x_max + e, y_max + e, z_max + e));
    return true;
}

Point3 Triangle::randomPointIn() const
{
    // Reflection method https://blogs.sas.com/content/iml/2020/10/19/random-points-in-triangle.html
    // Adapted for 3d.

    // TODO: not sure if this actually works

    Direction edge1 = m_points[1] - m_points[0];
    Direction edge2 = m_points[2] - m_points[0];

    double r1 = randomGen.getDouble();
    double r2 = randomGen.getDouble();

    if (r1 + r2 > 1)
    {
        r1 = 1 - r1;
        r2 = 1 - r2;
    }

    return (edge1 * r1 + edge2 * r2) + m_points[0];
}

double Triangle::pdf(const Ray &r) const
{
    HitRecord rec;
    if (!this->hit(r, 0.0001, inf, rec))
        return 0;

    // Should be 1/area of triangle

    Direction edge1 = m_points[1] - m_points[0];
    Direction edge2 = m_points[2] - m_points[0];

    double area = cross(edge1, edge2).length() / 2.0;

    // TODO: again not sure if this is correct

    return 1.0 / area;
}