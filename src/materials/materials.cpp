#include <core.h>
#include <random.h>
#include <vec3.h>
#include <algorithm>

#include <materials/lambertian.h>
#include <materials/metal.h>
#include <materials/dielectric.h>
#include <materials/pbr.h>

double schlickFresnel(double u)
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

/**
 * Cook Torrance BRDF
 * 
 * MAIN EQUATION: fr = kd*fl + ks*fc
 * 
 * ks: amount of reflected light
 * kd: amount of refracted light
 * kd = 1 - ks
 * 
 * ??? ks is also the fresnell term F ???
 * 
 * C = albedo color
 * fl = C / PI (lambertian diffuse)
 * 
 * D: Normal distribution function
 * G: Geometry function
 * fc = (D*G*F) / (4 * (l . n) * (v . n))
 * 
 * For D we choose the GGX distribution function
 * D = a^2 / (pi * ((n . h)^2 * (a^2 - 1) + 1)^2)
 * 
 * G will be the Sclick GGX approximation
 * G = Gs(n,v,k)*Gs(n,l,k)
 * Gs = (n.v) / ( (n.v) * (1 - k) + k )
 * 
 * F is the Schlick fresnell approximation
*/

double distributionGGX(Direction n, Direction h, double roughness) {
    double a = roughness*roughness;
    double a2 = a*a;
    double ndoth = dot(n, h);
    double term = ((ndoth*ndoth) * (a2 - 1) + 1);
    return a2 / (pi * term*term);
}

double geometrySchlickGGX(double ndotv, double roughness) {
    double r = roughness + 1;
    double k = r*r / 8.0;
    return ndotv / (ndotv*(1 - k) + k);
}

double geometrySmith(Direction n, Direction v, Direction l, double roughness) {
    double ndotv = fmax(dot(n, v), 0.0);
    double ndotl = fmax(dot(n, l), 0.0);
    double ggx2  = geometrySchlickGGX(ndotv, roughness);
    double ggx1  = geometrySchlickGGX(ndotl, roughness);

    return ggx1 * ggx2;
}


bool Pbr::scatter(const Ray &ray, const HitRecord &rec, Color &attenuation, Ray &scattered) const {
#if 0
    Direction reflected = reflect(normalize(ray.direction()), rec.normal);
    scattered = Ray(rec.p, reflected + m_roughness * randomInUnitSphere());
    attenuation = m_baseColor;
    return (dot(scattered.direction(), rec.normal) > 0);
#endif

    Direction n = normalize(rec.normal);
    //Direction scatter_direction = reflect(normalize(ray.direction()), n) + m_roughness * randomInUnitSphere();
    Direction scatter_direction = normalize(n + normalize(randomInUnitSphere()));
     
    // Check if the scatter direction is valid and not near zero
    if (scatter_direction.isNearZero())
        scatter_direction = n;

    // Calculate the F0 term, use the average F0 of 0.04 for dielectric materials
    // and calculate a linear interpolation between that average and the material color
    // if the material is not dielectric.
    Color F0 = lerp(Color(0.04), m_baseColor, m_metalic);

    // Cook Torrance BRDF
    Direction l = scatter_direction;
    Direction v = ray.direction();

    Direction h = normalize(v + l);
    double NDF = distributionGGX(n, h, m_roughness);
    double G = geometrySmith(n, v, l, m_roughness);
    
    double ndotv = dot(n, v);
    double ndotl = dot(n, l);

    Color F = F0 + (Color(1) - F0) * schlickFresnel(fmax(dot(v, h), 0));

    Color ks = F;
    Color kd = Color(1) - ks;
    kd *= 1 - m_metalic;

    Color fs = (NDF * G * F) / (4 * fmax(ndotv, 0) * fmax(ndotl, 0) + 0.000001);
    Color fd = kd / pi;

    attenuation = m_baseColor * (fd + fs) * fmax(ndotl, 0);
    scattered = Ray(rec.p, scatter_direction);

    return true;
}

bool Pbr::emitted(double u, double v, const Point3 &p, Color& emission) const {
    emission = m_emission * m_emission_strength;
    
    return true;
}