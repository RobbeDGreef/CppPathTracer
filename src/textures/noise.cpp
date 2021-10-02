#include <textures/noise_texture.h>

#include <random.h>

PerlinNoise::PerlinNoise()
{
    m_ranfloat = new double[m_point_count];

    for (int i = 0; i < m_point_count; i++)
        m_ranfloat[i] = randomDouble();

    for (int i = 0; i < 3; i++)
        m_perm[i] = generatePerm();
}

int *PerlinNoise::generatePerm()
{
    int *perm = new int[m_point_count];

    for (int i = 0; i < m_point_count; i++)
        perm[i] = i;
    
    permute(perm, m_point_count);

    return perm;
}

void PerlinNoise::permute(int *perm, int n)
{
    for (int i = n - 1; i > 0; --i)
    {
        int target = randomInt(0, i);
        int tmp = perm[i];
        perm[i] = perm[target];
        perm[target] = tmp;
    }
}

double PerlinNoise::noise(const Point3 &p) const
{
    auto i = static_cast<int>(4 * p.x()) & 255;
    auto j = static_cast<int>(4 * p.y()) & 255;
    auto k = static_cast<int>(4 * p.z()) & 255;

    return m_ranfloat[m_perm[0][i] ^ m_perm[1][j] ^ m_perm[2][k]];
}