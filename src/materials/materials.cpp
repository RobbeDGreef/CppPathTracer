#include <core.h>
#include <random.h>
#include <vec3.h>
#include <algorithm>
#include <onb.h>

#include <materials/lambertian.h>
#include <materials/metal.h>
#include <materials/dielectric.h>
#include <materials/pbr.h>

#if 0

bool Lambertian::scatter(const Ray &ray, const HitRecord &rec, Color &attenuation, Ray &scattered) const
{
    Direction scatter_direction = rec.normal + normalize(randomInUnitSphere());
    
    // Check if the scatter direction is valid and not near zero
    if (scatter_direction.isNearZero())
        scatter_direction = rec.normal;

    scattered = Ray(rec.p, scatter_direction);
    attenuation = m_texture->value(rec.u, rec.v, rec.p);
    return true;
}

bool Metal::scatter(const Ray &ray, const HitRecord &rec, Color &attenuation, Ray &scattered) const
{
    Direction reflected = reflect(normalize(ray.direction()), rec.normal);
    scattered = Ray(rec.p, reflected + m_fuzzyness * randomInUnitSphere());
    attenuation = m_albedo;
    return (dot(scattered.direction(), rec.normal) > 0);
}

bool Dielectric::scatter(const Ray &ray, const HitRecord &rec, Color &attenuation, Ray &scattered) const
{
    attenuation = Color(1.0, 1.0, 1.0);
    double refraction_ratio = rec.front_face ? (1.0 / m_refraction_index) : m_refraction_index;
    
    Direction unit_direction = normalize(ray.direction());
    double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
    double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    // Instead of sending out two rays, one for refraction and one for 
    // reflection we just randomly switch between the two
    Direction direction;
    bool cannot_refract = refraction_ratio * sin_theta > 1.0;
    if (cannot_refract || reflectance(cos_theta, refraction_ratio) > randomDouble())
        direction = reflect(unit_direction, rec.normal);
    else
        direction = refract(unit_direction, rec.normal, refraction_ratio);    

    scattered = Ray(rec.p, direction);

    return true;
}
#endif

#if 0
Color Pbr::color(const Ray &ray, const HitRecord &rec, const Ray &light_ray) const
{

    const Color base = m_baseColor->value(rec.u, rec.v, rec.p);

    // Metallic materials use the base color as attenuation to the reflected ray.
    // if (m_metallic)
    //{
    //    return base;
    //}

    // Non material or dielectric material.
    // Calculate diffuse and specular terms

    const Direction v = normalize(-ray.direction());
    const Direction l = normalize(light_ray.direction());
    const Direction n = normalize(rec.normal);
    double ndotl = dot(n, l);

    return BRDFCookTorrance(v, l, n, m_metallic, m_roughness, base) * fmax(ndotl, 0);
}


bool Pbr::scatter(const Ray &ray, const HitRecord &rec, Ray &scatter, Color attenuation, double& pdf) const
{

    Direction n = normalize(rec.normal);
#if 0
    if (m_metallic) {
        // Metallic materials reflect the light along the normal, we add a random
        // term to simulate a rough metallic surface.
        Direction reflected = reflect(normalize(ray.direction()), n);
        scatter = Ray(rec.p, reflected + m_roughness * randomInUnitSphere());

        // The random in unit sphere could create a ray that scatters into the material
        // to overcome this we flip the direction vector
        if (dot(scatter.direction(), n) < 0) {
            scatter = Ray(rec.p, -scatter.direction());
        }
    }
#endif

#if 0
    // TODO this is code duplication
    const Color base = m_baseColor->value(rec.u, rec.v, rec.p);
    Color F0 = lerp(Color(0.04), base, m_metallic);

    // TODO: what to do with this reflectance
    Color reflectance;

    Direction out = sampleGGX(-ray.direction(), n, m_roughness, F0, reflectance);

    scatter = Ray(rec.p, out);
#endif

#if 0
    // Lambertian diffuse

    Direction scatter_direction = rec.normal + normalize(randomInUnitSphere());
    
    // Check if the scatter direction is valid and not near zero
    if (scatter_direction.isNearZero())
        scatter_direction = rec.normal;

    scattered = Ray(rec.p, scatter_direction);
    attenuation = m_texture->value(rec.u, rec.v, rec.p);
    return true;

#endif

#if 1
    ONB base(n);

    double r1 = randomDouble();
    double r2 = randomDouble();

    double phi = 2 * pi * r1;
    Direction dir = Direction(cos(phi)*sqrt(r2), sin(phi)*sqrt(r2), sqrt(1-r2));
    
    pdf = dot(base.w(), dir) / pi;
    scatter = Ray(rec.p, dir);

#endif

    return true;
}

double Pbr::pdf(const Ray &in, const Ray &out, const HitRecord &rec) const
{
    return dot(-in.direction(), rec.normal) * pi;
}

bool Pbr::emitted(double u, double v, const Point3 &p, Color &emission) const
{
    emission = m_emission->value(u, v, p) * m_emission_strength;

    return true;
}

#endif
