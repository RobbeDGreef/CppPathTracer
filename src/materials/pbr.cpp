#include <core.h>
#include <materials/pbr.h>
#include <random.h>
#include <materials/helpers.h>
#include <pdfs/cosinepdf.h>
#include <pdfs/uniformpdf.h>
#include <vec3.h>
#include <pdfs/ggxpdf.h>


#define USE_COOK_TORRANCE 1

#if 1


bool PBR::scatter(const Ray &ray, const HitRecord &rec, ScatterRecord &scatter) const
{
    // TODO: may have to be removed once nan/inf issues are resolved
    if (m_emission_strength > 0.01) {
        return false;
    }


    m_brdf.scatter(ray, rec, scatter);

    return true;
}

bool PBR::emitted(double u, double v, const Point3 &p, Color &emission) const
{
    emission = m_emission->value(u, v, p) * m_emission_strength;

    return true;
}

Color PBR::eval(const Ray &in, const HitRecord &rec, const ScatterRecord &srec) const 
{

    Color out = m_brdf.evaluate(in, rec, srec) * dot(normalize(rec.normal), normalize(srec.scattered_ray.direction()));

    return out;
}
#else
bool PBR::scatter(const Ray &ray, const HitRecord &rec, ScatterRecord &scatter) const
{
    // TODO: may have to be removed once nan/inf issues are resolved
    if (m_emission_strength > 0.01) {
        return false;
    }

    Direction n = normalize(rec.normal);
    Direction v = normalize(-ray.direction());

    if (m_metallic) {
        scatter.skip_pdf = true;
        scatter.scattered_ray = Ray(rec.p, metalScatter(n, v, m_roughness));
        return true;
    }

    // Non-metallic / dielectric ray

    // TODO: add transmission
    scatter.skip_pdf = false;
    scatter.pdf = std::make_shared<GGXPDF>(ray, n, m_roughness, 0.04);
    //scatter.pdf = std::make_shared<CosinePDF>(rec.normal);

    return true;
}

bool PBR::emitted(double u, double v, const Point3 &p, Color &emission) const
{
    emission = m_emission->value(u, v, p) * m_emission_strength;

    return true;
}

Color PBR::eval(const Ray &in, const HitRecord &rec, const ScatterRecord &srec) const 
{
    Color base = m_baseColor->value(rec.u, rec.v, rec.p);

#if 0

    if (m_metallic) {
        return base;
    }

    double cos_theta = dot(normalize(rec.normal), normalize(srec.scattered_ray.direction()));
    if (cos_theta < 0) return 0;

    return base * cos_theta / pi;
#endif

    Direction v = normalize(-in.direction());
    Direction l = normalize(srec.scattered_ray.direction());
    Direction n = normalize(rec.normal);
    return BRDFCookTorrance(v, l, n, m_metallic, m_roughness, base) * dot(n, l);
}

#endif
