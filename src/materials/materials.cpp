#include <core.h>
#include <random.h>
#include <vec3.h>
#include <algorithm>

#include <materials/lambertian.h>
#include <materials/metal.h>
#include <materials/dielectric.h>

double Material::schlickFresnel(double u)
{
    return pow(std::clamp(1.0-u, 0.0, 1.0), 5);
}

static Direction reflect(const Direction v, const Direction n)
{
    // reflected ray direction should be: v + 2b
    // and b = v * n
    // Since we want the reflection to point in we just do v - 2b
    return v - 2 * dot(v, n) * n;
}

static Direction refract(const Direction &uv, const Direction &n, double refract_ratio)
{
    // Snell's law
    auto cos_theta = fmin(dot(-uv, n), 1.0);
    Direction r_out_perp = refract_ratio * (uv + cos_theta*n);
    Direction r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

static double reflectance(double cos, double refract_index)
{
    // Schlick's approximation for reflectance
    // R(theta) = R0 + (1 - R0)(1 - cos(theta))^5
    // with
    // R0 = ((n1 - n2) / (n1 + n2))^2
    double r0 = (1 - refract_index) / (1 + refract_index);
    r0 *= r0;
    return r0 + (1 - r0) * pow(1-cos, 5);
}

bool Lambertian::scatter(const Ray &ray, const HitRecord &rec, Color &attenuation, Ray &scattered) const
{
    Direction scatter_direction = rec.normal + normalize(randomInUnitSphere());
    
    // Check if the scatter direction is valid and not near zero
    if (scatter_direction.isNearZero())
        scatter_direction = rec.normal;

    scattered = Ray(rec.p, scatter_direction, ray.timeframe());
    attenuation = m_texture->value(rec.u, rec.v, rec.p);
    return true;
}

bool Metal::scatter(const Ray &ray, const HitRecord &rec, Color &attenuation, Ray &scattered) const
{
    Direction reflected = reflect(normalize(ray.direction()), rec.normal);
    scattered = Ray(rec.p, reflected + m_fuzzyness * randomInUnitSphere(), ray.timeframe());
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

    scattered = Ray(rec.p, direction, ray.timeframe());

    return true;
}