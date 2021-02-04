#pragma once

#include <vec3.h>
#include <textures/texture.h>

class PerlinNoise
{
private:
    const int m_point_count = 256;
    double *m_ranfloat;
    int *m_perm[3];

private:
    int* generatePerm();
    void permute(int *, int);

public:
    PerlinNoise();
    double noise(const Point3 &p) const;
};

class NoiseTexture : public Texture
{
private:
    PerlinNoise m_noise_gen;

public:
    NoiseTexture() {}

    Color value(double u, double v, const Point3 &p) const override
    {
        return m_noise_gen.noise(p) * Color(1,1,1);
    }
};