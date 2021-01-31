#include <random.h>
#include <vec3.h>

// Easily the fastest of the two
#if 1
double randomDouble()
{
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}
#endif

#if 0
double randomDouble()
{
    return rand() / (RAND_MAX + 1.0);
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