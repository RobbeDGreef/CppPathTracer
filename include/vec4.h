#pragma once

#include <vec3.h>

template<class T>
class Vec4
{
private:
    T e[4] = {0, 0, 0, 0};

public:
    Vec4() {}
    constexpr Vec4(T x, T y, T z, T w) : e{x, y, z, w} {}

    Vec4(T x) : Vec4(x, x, x, x) {}

    T x() const { return e[0]; }
    T y() const { return e[1]; }
    T z() const { return e[2]; }
    T w() const { return e[3]; }

    T operator[](int i) const { return e[i]; } 
    Vec4 operator-() const { return Vec3(-e[0], -e[1], -e[2], -e[3]); }
    
    void set(int i, T val)
    {
        e[i] = val;
    }

};

template<class T>
inline Vec3<T> operator*(const Vec4<T> &q, const Vec3<T> &v)
{
    T ix, iy, iz, iw;
    ix =  q.w() * v.x() + q.y() * v.z() + q.z() * v.y();
    iy =  q.w() * v.y() + q.z() * v.x() - q.x() * v.z();
    iz =  q.w() * v.z() + q.x() * v.y() - q.y() * v.x();
    iw = -q.x() * v.x() - q.y() * v.y() - q.z() * v.z();

    Vec3<T> out;
    out.set(0, ix * q.w() + iw * -q.x() + iy * -q.z() - iz * -q.y());
    out.set(1, iy * q.w() + iw * -q.y() + iz * -q.x() - ix * -q.z());
    out.set(2, iz * q.w() + iw * -q.z() + ix * -q.y() - iy * -q.x());

    return out;
}

using Quaternion = Vec4<double>;