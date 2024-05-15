#pragma once

#include <vec3.h>

double reflectance(double cos, double refract_index);
Direction refract(const Direction &uv, const Direction &n, double refract_ratio);
Direction reflect(const Direction v, const Direction n);

template <typename T>
T schlickFresnel(T F0, double u)
{
    return F0 + (T(1) - F0) * pow(std::clamp(1.0 - u, 0.0, 1.0), 5);
}

double distributionGGX(Direction n, Direction h, double roughness);
double geometrySchlickGGX(double ndotv, double roughness);
double geometrySmith(Direction n, Direction v, Direction l, double roughness);
Point3 sphericalToCartesian(double theta, double phi);
Direction sampleGGX(const Direction& view, const Direction& normal, double roughness, Color F0, Color &reflectance);
Color BRDFCookTorrance(const Direction v, const Direction l, const Direction n, double metallic, double roughness, Color c);
Direction lambertianScatter(const Direction& normal);
Direction metalScatter(const Direction& normal, const Direction& in, double fuzzyness);
Point3 sphericalToCartesian(double theta, double phi);