#include <hitable/triangle.h>

void Triangle::getUV(const Point3 &p, double &u, double &v) const
{
    u = 0;
    v = 0;
}

bool Triangle::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const 
{
    // Möller-Trumbore intersection algorithm
    const double elipson = 0.000001;
    Direction edge1, edge2, h, s, q;
    double a, f, u, v;

    edge1 = m_points[1] - m_points[0];
    edge2 = m_points[2] - m_points[0];
    h = cross(r.direction(), edge2);
    a = dot(edge1, h);

    // Ray is parallel to the triangle
    if (a > -elipson && a < elipson)
        return false;
    
    f = 1.0 / a;
    s = r.origin() - m_points[0];
    u = f * dot(s, h);

    if (u < 0.0 || u > 1.0)
        return false;
    
    q = cross(s, edge1);
    v = f * dot(r.direction(), q);

    if (v < 0.0 || u + v > 1.0)
        return false;

    // At this point we can compute t
    double t = f * dot(edge2, q);
    if (t > elipson) 
    {
        // Ray intersection needs to be in range
        if (t < t_min || t > t_max)
            return false;

        rec.p = r.at(t);
        Direction outward_normal = normalize(cross(edge1, edge2));
        rec.set_face_normal(r, outward_normal);
        rec.mat = material(r.timeframe());
        getUV(rec.p, rec.u, rec.v);
        return true;
    }
    else
    {   
        // We have a line intersection but not a ray intersection
        return false;
    }
}

bool Triangle::boundingBox(double t0, double t1, AABB &bounding_box) const 
{
    // Is there a better way?
    double z_min = fmin(fmin(m_points[0].z(), m_points[1].z()), m_points[2].z());
    double z_max = fmax(fmax(m_points[0].z(), m_points[1].z()), m_points[2].z());
    double x_min = fmin(fmin(m_points[0].x(), m_points[1].x()), m_points[2].x());
    double x_max = fmax(fmax(m_points[0].x(), m_points[1].x()), m_points[2].x());
    double y_min = fmin(fmin(m_points[0].y(), m_points[1].y()), m_points[2].y());
    double y_max = fmax(fmax(m_points[0].y(), m_points[1].y()), m_points[2].y());

    bounding_box = AABB(Point3(x_min, y_min, z_min), Point3(x_max, y_max, z_max));
    return true;
}