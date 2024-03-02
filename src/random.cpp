#include <random.h>
#include <vec3.h>
#include <core.h>

#if 0
double randomDouble()
{
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}
#endif

#if 1
double randomDouble()
{
    // Xoroshiro128+
    static uint64_t shuffle[2] = {123456789, 987654321};
    uint64_t s1 = shuffle[0];
    uint64_t s0 = shuffle[1];
    uint64_t res = s0 + s1;
    shuffle[0] = s0;
    s1 ^= s1 << 23;
    shuffle[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);
    return (double) res / UINT64_MAX;
}
#endif


#if 0
// 0.947 - 0.967
double randomDouble()
{
    static uint64_t x=123456789, y=362436069, z=521288629;
    register unsigned long t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;

    return (double) z / UINT64_MAX;
}
#endif


double randomDouble(double min, double max)
{
    return min + (max-min) * randomDouble();
}


int randomInt(int min, int max)
{
    return static_cast<int>(randomDouble(min, max+0.999));
}

Point3 randomInUnitSphere()
{
    while (true)
    {
        Point3 p = randomVec3(-1, 1);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

Point3 randomInUnitDisk()
{
    while (true)
    {
        Point3 p = Point3(randomDouble(-1,1), randomDouble(-1,1), 0);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}