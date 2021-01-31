#pragma once

#include <random>
#include <vec3.h>

double randomDouble();
double randomDouble(double min, double max);
Vec3<double> randomInUnitSphere();
Vec3<double> randomInUnitDisk();

int randomInt(int min, int max);

inline Vec3<double> randomVec3(double min, double max)
{
    return Vec3<double>(randomDouble(min, max), randomDouble(min, max), randomDouble(min, max));
}

inline Vec3<double> randomVec3()
{
    return Vec3<double>(randomDouble(), randomDouble(), randomDouble());
}