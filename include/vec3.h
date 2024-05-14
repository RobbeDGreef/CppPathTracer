#pragma once

#include <core.h>

template <class T>
class Vec3
{
private:
    T e[3] = {0, 0, 0};

public:
    Vec3() {}
    constexpr Vec3(T x, T y, T z) : e{x, y, z} {}

    Vec3(T x) : Vec3(x, x, x) {}

    T x() const { return e[0]; }
    T y() const { return e[1]; }
    T z() const { return e[2]; }

    T operator[](int i) const { return e[i]; }
    Vec3 operator-() const { return Vec3(-e[0], -e[1], -e[2]); }

    void set(int i, T val)
    {
        e[i] = val;
    }

    Vec3 &operator+=(const Vec3 &v)
    {
        e[0] += v[0];
        e[1] += v[1];
        e[2] += v[2];
        return *this;
    }

    Vec3 &operator-=(const Vec3 &v)
    {
        return *this += -v;
    }

    Vec3 &operator*=(double v)
    {
        e[0] *= v;
        e[1] *= v;
        e[2] *= v;
        return *this;
    }

    Vec3 &operator/=(double v)
    {
        return *this *= 1 / v;
    }

    bool isNearZero() const
    {
        const double s = 1e-8;
        return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
    }

    double length() const
    {
        return sqrt(length_squared());
    }

    double length_squared() const
    {
        return (double)(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
    }

    std::string to_string() const
    {
        return std::to_string(e[0]) + " " + std::to_string(e[1]) + " " + std::to_string(e[2]);
    }
};

using Point3 = Vec3<double>;
using Color = Vec3<double>;
using Direction = Vec3<double>;

template <class T>
inline Vec3<T> operator+(const Vec3<T> &x, const Vec3<T> &y)
{
    return Vec3<T>(x.x() + y.x(), x.y() + y.y(), x.z() + y.z());
}

template <class T>
inline Vec3<T> operator-(const Vec3<T> &x, const Vec3<T> &y)
{
    return Vec3<T>(x.x() - y.x(), x.y() - y.y(), x.z() - y.z());
}

template <class T>
inline Vec3<T> operator*(const Vec3<T> &x, const Vec3<T> &y)
{
    return Vec3<T>(x.x() * y.x(), x.y() * y.y(), x.z() * y.z());
}

template <class T>
inline Vec3<T> operator*(double v, const Vec3<T> &x)
{
    return Vec3<T>(x.x() * v, x.y() * v, x.z() * v);
}

template <class T>
inline Vec3<T> operator*(const Vec3<T> &x, double v)
{
    return Vec3<T>(x.x() * v, x.y() * v, x.z() * v);
}

template <class T>
inline Vec3<T> operator/(Vec3<T> x, double v)
{
    return (1 / v) * x;
}

template <class T>
inline double dot(const Vec3<T> &x, const Vec3<T> &y)
{
    return x.x() * y.x() + x.y() * y.y() + x.z() * y.z();
}

template <class T>
inline Vec3<T> cross(const Vec3<T> &x, const Vec3<T> &y)
{
    return Vec3<T>(x.y() * y.z() - x.z() * y.y(),
                   x.z() * y.x() - x.x() * y.z(),
                   x.x() * y.y() - x.y() * y.x());
}

template <class T>
inline Vec3<T> normalize(Vec3<T> v)
{
    return v / v.length();
}

template <class T>
inline Vec3<T> clamp(Vec3<T> v, double min, double max)
{
    for (int i = 0; i < 3; ++i)
    {
        v.set(i, clamp(v[i], min, max));
    }
    return v;
}

template <class T>
inline Vec3<T> sqrt(Vec3<T> v)
{
    for (int i = 0; i < 3; ++i)
    {
        v.set(i, sqrt(v[i]));
    }
    return v;
}

inline Vec3<double> minValues(Point3 x, Point3 y)
{
    Point3 ret;
    for (int i = 0; i < 3; i++)
    {
        ret.set(i, fmin(x[i], y[i]));
    }

    return ret;
}

inline Vec3<double> maxValues(Point3 x, Point3 y)
{
    Point3 ret;
    for (int i = 0; i < 3; i++)
    {
        ret.set(i, fmax(x[i], y[i]));
    }

    return ret;
}

template <class T>
inline T lerp(T a, T b, double t)
{
    return a + (b - a) * t;
}

template <class T>
inline Vec3<T> fabs(Vec3<T> a)
{
    return Vec3(fabs(a[0]), fabs(a[1]), fabs(a[2]));
}