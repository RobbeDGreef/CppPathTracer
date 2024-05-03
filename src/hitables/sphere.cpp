#include <hitables/sphere.h>

bool Sphere::hit(const Ray &ray, double t_min, double t_max, HitRecord &rec) const
{
    // Basically: x^2 + y^2 + z^2 = r^2 -> (x - cx)^2 + (y - cy)^2 + (z - cz)^2 = r^2
    // We take C as our center (cx, cy, cz) and P as our (x y z) so then we get
    // (P - C) * (P - C) = r^2
    // since P is actually a function of t we get.
    // (P(t) - C) * (P(t) - C) = r^2
    // and now when we substitute our P(t) formula we get
    // (A + tB - C) * (A + tB - C) = r^2
    // and that is equal to
    // t^2*b^2 + 2tb * (A - C) + (A - C) * (A - C) - r^2 = 0
    // Which is just a quadratic formula for t. So we just solve that here
    // Then after some simplifying (like vector1 * vector1 is just length squared)
    // and removing our 2.0 factor we get:
    Point3 oc = ray.origin() - center();
    double a = ray.direction().length_squared();
    double half_b = dot(oc, ray.direction());
    double c = dot(oc, oc) - radius() * radius();
    double d = half_b*half_b - a*c;
    if (d < 0)
        return false;

    double sqrt_d = sqrt(d);
    
    // Find an acceptable root within range
    double root = (-half_b - sqrt_d) / a;

    if (root < t_min || root > t_max)
    {
        root = (-half_b + sqrt_d) / a;
        if (root < t_min || root > t_max)
            return false;
    }

    rec.t = root;
    rec.p = ray.at(root);
    Direction outward_normal = (rec.p - center()) / radius();
    rec.set_face_normal(ray, outward_normal);
    rec.mat = material();
    getUV(outward_normal, rec.u, rec.v);
    
    return true;
}

bool Sphere::boundingBox(AABB &bounding_box) const
{
    return true;
}

void Sphere::getUV(const Point3 &p, double &u, double &v)
{
    // phi = atan2(-z, x) + PI
    // theta = acos(-y)
    // u and v have to be [0, 1] so we will have to normalise them like:
    // u = phi / 2pi
    // v = theta / pi

    u = (atan2(-p.z(), p.x()) + pi) / (2 * pi);
    v = acos(-p.y()) / pi;
}