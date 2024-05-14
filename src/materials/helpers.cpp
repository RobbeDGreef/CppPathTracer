#include <vec3.h>
#include <onb.h>
#include <random.h>

Color schlickFresnel(Color F0, double u)
{
    return F0 + (Color(1) - F0) * pow(std::clamp(1.0 - u, 0.0, 1.0), 5);
}
Direction reflect(const Direction v, const Direction n)
{
    // reflected ray direction should be: v + 2b
    // and b = v * n
    // Since we want the reflection to point in we just do v - 2b
    return v - 2 * dot(v, n) * n;
}

Direction refract(const Direction &uv, const Direction &n, double refract_ratio)
{
    // Snell's law
    auto cos_theta = fmin(dot(-uv, n), 1.0);
    Direction r_out_perp = refract_ratio * (uv + cos_theta * n);
    Direction r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

double reflectance(double cos, double refract_index)
{
    // Schlick's approximation for reflectance
    // R(theta) = R0 + (1 - R0)(1 - cos(theta))^5
    // with
    // R0 = ((n1 - n2) / (n1 + n2))^2
    double r0 = (1 - refract_index) / (1 + refract_index);
    r0 *= r0;
    return r0 + (1 - r0) * pow(1 - cos, 5);
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

double distributionGGX(Direction n, Direction h, double roughness)
{
    double a = roughness * roughness;
    double a2 = a * a;
    double ndoth = dot(n, h);
    double term = ((ndoth * ndoth) * (a2 - 1) + 1);
    return a2 / (pi * term * term);
}

double geometrySchlickGGX(double ndotv, double roughness)
{
    double r = roughness + 1;
    double k = r * r / 8.0;
    return ndotv / (ndotv * (1 - k) + k);
}

double geometrySmith(Direction n, Direction v, Direction l, double roughness)
{
    double ndotv = fmax(dot(n, v), 0.0);
    double ndotl = fmax(dot(n, l), 0.0);
    double ggx2 = geometrySchlickGGX(ndotv, roughness);
    double ggx1 = geometrySchlickGGX(ndotl, roughness);

    return ggx1 * ggx2;
}

inline Point3 sphericalToCartesian(double theta, double phi)
{
    double x = sinf(theta) * cosf(phi);
    double y = sinf(theta) * sinf(phi);
    double z = cosf(theta);
    return Point3(x, y, z);
}

Direction sampleGGX(const Direction& view, const Direction& normal, double roughness, Color F0, Color &reflectance)
{
    // FUTURE ROBBE: the issue here is that the base of these vectors should be orthonormal

    ONB base(normal);
    Direction v = base.local(view);
    Direction n = base.local(normal);

    // TODO: see https://schuttejoe.github.io/post/ggximportancesamplingpart2/ for better sampler
    double r0 = randomGen.getDouble();
    double r1 = randomGen.getDouble();
    double a2 = roughness * roughness;
    double theta = acosf(sqrtf((1 - r0) / ((a2 - 1) * r0 + 1)));
    double phi = 2 * pi * r1;
    // H is the microfacet normal
    Direction h = sphericalToCartesian(theta, phi);

    Direction out = 2 * h * dot(h, v) - v;

    double ndotv = dot(n, v);
    double ndoth = dot(n, h);
    double vdoth = dot(v, h);
    
    if (dot(n, out) > 0.0 && dot(out, h) > 0.0) {
        Color F = schlickFresnel(F0, fmax(dot(v, h), 0));
        double G = geometrySmith(n, v, out, roughness);
        double weight = fabs(vdoth) / (ndotv * ndoth);
        reflectance = F * G * weight;
    }

    return base.local(out);
}

Color BRDFCookTorrance(const Direction v, const Direction l, const Direction n, double metallic, double roughness, Color c)
{
    // The F0 term, use the average F0 of 0.04 for dielectric materials
    Color F0 = lerp(Color(0.04), c, metallic);

    Direction h = normalize(v + l);
    double NDF = distributionGGX(n, h, roughness);
    double G = geometrySmith(n, v, l, roughness);

    double ndotv = dot(n, v);
    double ndotl = dot(n, l);

    Color F = schlickFresnel(F0, fmax(dot(v, h), 0));

    Color ks = F;
    Color kd = Color(1) - ks;
    kd *= 1 - metallic;

    Color fs = (NDF * G * F) / (4 * fmax(ndotv, 0) * fmax(ndotl, 0) + 0.000001);
    Color fd = kd * c / pi;

    return (fd + fs);
}

Direction lambertianScatter(const Direction& normal)
{
    Direction scatter_direction = normal + normalize(randomGen.getPoint3InUnitSphere());
    
    // Check if the scatter direction is valid and not near zero
    if (scatter_direction.isNearZero())
        scatter_direction = normal;

    return scatter_direction;
}

Direction metalScatter(const Direction& normal, const Direction& in, double fuzzyness)
{
    Direction reflected = reflect(normalize(in), normal);
    return reflected + fuzzyness * randomGen.getPoint3InUnitSphere();
}
