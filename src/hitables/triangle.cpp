#include <hitables/triangle.h>
#include <random.h>
#include <core.h>

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