#pragma once

#include <random>
#include <vec3.h>

#define USE_RAND 0
#define USE_XOROSHIRO128 0
#define USE_XOSHIRO256SS 1

class RandomGenerator
{
public:
#if USE_RAND

    RandomGenerator()
    {
        srand(0);
    }

    uint64_t getUint64()
    {
        uint32_t r1 = rand();
        uint32_t r2 = rand();
        uint32_t r3 = rand();

        // This is not really portable since log2 is only constexpr under gcc and not
        // under clang but idc I use gcc.
        constexpr int rand_increment = std::floor(std::log2(RAND_MAX));

        uint64_t res = 0;
        for (int i = 0; i < 64; i += rand_increment)
        {
            res <<= rand_increment;
            res ^= rand();
        }

        return res;
    }

#endif

#if USE_XOROSHIRO128
    uint64_t getUint64()
    {
        // Xoroshiro128+
        static uint64_t shuffle[2] = {123456789, 987654321};
        uint64_t s1 = shuffle[0];
        uint64_t s0 = shuffle[1];
        uint64_t res = s0;
        shuffle[0] = s0;
        s1 ^= s1 << 23;
        shuffle[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);
        return res;
    }
#endif
#if USE_XOSHIRO256SS

    /* see https://prng.di.unimi.it/xoshiro256starstar.c */

    uint64_t m_state[4];

    static inline uint64_t rotl(const uint64_t x, int k)
    {
        return (x << k) | (x >> (64 - k));
    }

    RandomGenerator()
    {
        std::ifstream file("/dev/urandom");
        if (!file.is_open()) {
            ERROR("Could not open /dev/urandom");
            exit(1);
        }

        file.read(reinterpret_cast<char*>(m_state), sizeof(uint64_t) * 4);

        file.close();
    }

    uint64_t getUint64()
    {
        const uint64_t result = rotl(m_state[1] * 5, 7) * 9;

        const uint64_t t = m_state[1] << 17;

        m_state[2] ^= m_state[0];
        m_state[3] ^= m_state[1];
        m_state[1] ^= m_state[2];
        m_state[0] ^= m_state[3];

        m_state[2] ^= t;

        m_state[3] = rotl(m_state[3], 45);

        return result;
    }

#endif

    double getDouble()
    {
        return (double)getUint64() / UINT64_MAX;
    }

    double getDouble(double min, double max)
    {
        return min + (max - min) * getDouble();
    }

    int getInt()
    {
        return getUint64();
    }

    int getInt(int min, int max)
    {
        return min + (max - min) * getDouble();
    }

    Vec3d getVec3Double()
    {
        return Vec3d(getDouble(), getDouble(), getDouble());
    }
    Vec3d getVec3Double(double min, double max)
    {
        return Vec3d(getDouble(min, max), getDouble(min, max), getDouble(min, max));
    }

    Point3 getPoint3()
    {
        return getVec3Double();
    }

    Point3 getPoint3InUnitSphere()
    {
        while (true)
        {
            Point3 p = getVec3Double(-1, 1);
            if (p.length_squared() >= 1)
                continue;
            return p;
        }
    }

    Point3 getPoint3InUnitDisk()
    {
        while (true)
        {
            Point3 p = Point3(getDouble(-1, 1), getDouble(-1, 1), 0);
            if (p.length_squared() >= 1)
                continue;
            return p;
        }
    }

    Point3 getPoint3InUnitDome(const Direction &n)
    {
        Point3 r = getPoint3InUnitSphere();

        if (dot(r, n) < 0)
        {
            r = -r;
        }

        return r;
    }
};

extern thread_local RandomGenerator randomGen;