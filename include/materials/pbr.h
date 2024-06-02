#pragma once

#include <textures/texture.h>
#include <hitables/hitable.h>
#include <materials/material.h>

#define USE_NEW_BRDF_BASED_SYSTEM 1

#if USE_NEW_BRDF_BASED_SYSTEM

#include <materials/helpers.h>
#include <pdfs/mixturepdf.h>
#include <pdfs/cosinepdf.h>
#include <pdfs/ggxpdf.h>
#include <pdfs/uniformpdf.h>
#include <pdfs/fresnelpdf.h>

class BRDF
{
public:
    template <typename T>
    T mix(T a, T b, double v) const
    {
        return lerp(a, b, v);
    }

    virtual void scatter(const Ray &ray, const HitRecord &rec, ScatterRecord &srec) const = 0;
    virtual Color evaluate(const Ray &in, const HitRecord &rec, const ScatterRecord &srec) const = 0;
};

class MixBRDF : public BRDF
{
private:
    BRDF &m_brdf1;
    BRDF &m_brdf2;
    double m_mix;

public:
    MixBRDF(double mix, BRDF &brdf1, BRDF &brdf2)
        : m_brdf1(brdf1), m_brdf2(brdf2), m_mix(mix)
    {
    }

    Color evaluate(const Ray &in, const HitRecord &rec, const ScatterRecord &srec) const override
    {
        return mix(m_brdf1.evaluate(in, rec, srec), m_brdf2.evaluate(in, rec, srec), m_mix);
    }

    void scatter(const Ray &ray, const HitRecord &rec, ScatterRecord &srec) const override
    {
        ScatterRecord srec1, srec2;
        m_brdf1.scatter(ray, rec, srec1);
        m_brdf2.scatter(ray, rec, srec2);

        srec.skip_pdf = false;
        srec.pdf = std::make_shared<MixturePDF>(m_mix, srec1.pdf, srec2.pdf);
    }
};

class FresnelMixBRDF : public BRDF
{
private:
    double m_mix;
    BRDF &m_brdf1;
    BRDF &m_brdf2;

public:
    FresnelMixBRDF(double mix, BRDF &brdf1, BRDF &brdf2)
        : m_mix(mix), m_brdf1(brdf1), m_brdf2(brdf2) {}

    Color evaluate(const Ray &in, const HitRecord &rec, const ScatterRecord &srec) const override
    {
        Direction v = normalize(-in.direction());
        Direction l = normalize(srec.scattered_ray.direction());
        Direction h = normalize(v + l);
        double vdoth = dot(v, h);

        double fr = schlickFresnel(0.04, fabs(vdoth));
        return mix(m_brdf1.evaluate(in, rec, srec), m_brdf2.evaluate(in, rec, srec), fr);
    }

    void scatter(const Ray &ray, const HitRecord &rec, ScatterRecord &srec) const override
    {
        srec.skip_pdf = false;

        ScatterRecord srec1, srec2;
        m_brdf1.scatter(ray, rec, srec1);
        m_brdf2.scatter(ray, rec, srec2);

        // srec.pdf = std::make_shared<FresnelPDF>(m_mix, normalize(-ray.direction()), srec1.pdf, srec2.pdf);

        // TODO: this is incorrect as PDF
        srec.pdf = std::make_shared<MixturePDF>(0.5, srec1.pdf, srec2.pdf);
    }
};

class DiffuseBRDF : public BRDF
{
private:
    std::shared_ptr<Texture> m_color;

public:
    DiffuseBRDF(std::shared_ptr<Texture> color) : m_color(color) {}

    Color evaluate(const Ray &in, const HitRecord &rec, const ScatterRecord &srec) const override
    {
        Color base = m_color->value(rec.u, rec.v, rec.p);

        double cos_theta = dot(normalize(rec.normal), normalize(srec.scattered_ray.direction()));
        if (cos_theta < 0)
            return 0;

        return base * cos_theta / pi;
    }

    void scatter(const Ray &ray, const HitRecord &rec, ScatterRecord &srec) const override
    {
        srec.skip_pdf = false;
        srec.pdf = std::make_shared<CosinePDF>(rec.normal);
    }
};

class SpecularBRDF : public BRDF
{
private:
    double m_roughness;

    double calcV(Direction n, Direction h, Direction v_or_l) const
    {
        double a = m_roughness * m_roughness;
        double a2 = a * a;
        double d = dot(n, v_or_l);
        double nom = dot(h, v_or_l) > 0 ? 1 : 0;
        double denom = d + sqrt(a2 + (1 - a2) * d * d);
        return nom / denom;
    }

public:
    SpecularBRDF(double roughness)
    {
        m_roughness = roughness < 0.001 ? 0.001 : roughness;
    }

    Color evaluate(const Ray &in, const HitRecord &rec, const ScatterRecord &srec) const override
    {
        Direction n = normalize(rec.normal);
        Direction v = normalize(-in.direction());
        Direction l = normalize(srec.scattered_ray.direction());
        Direction h = normalize(v + l);

        // It does not make sense to get a H that is larger than perpendicular to the view angle
        // And you also dont want a H that is points down from the normal vector
        if (dot(v, h) < 0.0 || dot(n, h) < 0.0 || dot(n, l) < 0.0 || dot(n, v) < 0.0)
            return 0;

        double D = distributionGGX(n, h, m_roughness);
        double V = calcV(n, h, v) * calcV(n, h, l);

        return D * V;
    }

    void scatter(const Ray &ray, const HitRecord &rec, ScatterRecord &srec) const override
    {
        srec.skip_pdf = false;

        srec.pdf = std::make_shared<GGXPDF>(ray, rec.normal, m_roughness);
    }
};

class MetallicBRDF : public SpecularBRDF
{
private:
    std::shared_ptr<Texture> m_f0;

public:
    MetallicBRDF(double roughness, std::shared_ptr<Texture> f0) : SpecularBRDF(roughness), m_f0(f0) {}

    Color evaluate(const Ray &in, const HitRecord &rec, const ScatterRecord &srec) const override
    {
        return m_f0->value(rec.u, rec.v, rec.p) * SpecularBRDF::evaluate(in, rec, srec);
    }
};

#endif

class PBR : public Material
{
private:
    std::shared_ptr<Texture> m_baseColor;
    double m_roughness = 1;
    double m_metallic = 0.0;
    double m_transmission = 0.0;

    std::shared_ptr<Texture> m_emission;
    double m_emission_strength = 0;

    DiffuseBRDF m_diffuse_brdf;
    SpecularBRDF m_specular_brdf;
    FresnelMixBRDF m_dielectric_brdf;
    MetallicBRDF m_metallic_brdf;
    MixBRDF m_brdf;
    // BRDF &m_brdf = m_specular_brdf;
    // BRDF& m_brdf = m_diffuse_brdf;

public:
    PBR(const std::shared_ptr<Texture> &color, double roughness, bool metallic,
        double transmission, const std::shared_ptr<Texture> &emission, double emission_strength) : m_baseColor(color), m_roughness(roughness), m_metallic(metallic), m_transmission(transmission),
                                                                                                   m_emission(emission), m_emission_strength(emission_strength), m_diffuse_brdf(color),
                                                                                                   m_specular_brdf(roughness), m_dielectric_brdf(1 - m_roughness, m_diffuse_brdf, m_specular_brdf),
                                                                                                   m_metallic_brdf(roughness, m_baseColor), m_brdf(metallic, m_dielectric_brdf, m_metallic_brdf) {}
    bool scatter(const Ray &r, const HitRecord &rec, ScatterRecord &srec) const override;
    bool emitted(double u, double v, const Point3 &p, Color &emission) const override;
    Color eval(const Ray &in, const HitRecord &rec, const ScatterRecord &srec) const override;
};