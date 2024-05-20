#pragma once

#include <vec3.h>
#include <config.h>

class Ray
{
protected:
    Point3 m_origin;
    Direction m_dir;

    // This inverted direction is precomputed for each ray to 
    // speed up operations such as AABB hit testing.
    Direction m_inverted_dir;

public:
    Ray() {}
    Ray(const Point3 &origin, const Direction &direction)
        : m_origin(origin), m_dir(direction), m_inverted_dir(1.0 / direction) {}

    Point3 origin() const { return m_origin; }
    Direction direction() const { return m_dir; }
    Direction inverted_direction() const { return m_inverted_dir; }

    Point3 at(double t) const
    {
        // P(t) = A + tB where A is our origin and B is our directional component
        // t is the parameter which specifies where on the ray we exactly are.

        // Important: Notice that the t comes before our directional vector, this is
        //            important because the operator overload vec3 * double is actually
        //            destructive and changes the vector we give it. double * vec3 creates
        //            a new vector.
        return m_origin + t * m_dir;
    }
};
