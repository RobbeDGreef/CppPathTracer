#include <core.h>
#include <materials/uber.h>
#include <random.h>
#include <materials/helpers.h>
#include <pdfs/cosinepdf.h>
#include <vec3.h>

bool Uber::scatter(const Ray &ray, const HitRecord &rec, ScatterRecord &scatter) const
{
    // TODO: may have to be removed once nan/inf issues are resolved
    if (m_emission_strength > 0.01) {
        return false;
    }

    Color base = m_baseColor->value(rec.u, rec.v, rec.p);
    Direction n = normalize(rec.normal);
    Direction v = normalize(ray.direction());

    if (m_metallic) {
        scatter.attenuation = base;
        scatter.skip_pdf = true;
        scatter.scattered_ray = Ray(rec.p, metalScatter(n, v, m_roughness));
        return true;
    }

    // Non-metallic / dielectric ray

    // TODO: add transmission
    scatter.skip_pdf = false;
    scatter.pdf = std::make_shared<CosinePDF>(n);
    scatter.attenuation = base;

    return true;
}

bool Uber::emitted(double u, double v, const Point3 &p, Color &emission) const
{
    emission = m_emission->value(u, v, p) * m_emission_strength;

    return true;
}

double Uber::pdf(const Ray &in, HitRecord rec, const ScatterRecord &srec) const 
{
    double cos_theta = dot(normalize(rec.normal), normalize(srec.scattered_ray.direction()));
    if (cos_theta < 0) return 0;

    return cos_theta / pi;
}
