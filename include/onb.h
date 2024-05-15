#pragma once


#include <vec3.h>

class ONB
{
private:
    Direction m_axis[3];

    void buildFromNormal(const Direction &w)
    {
        // Calculate the unit vector of our new up vector w,
        // then calculate the perpendicular vectors on this w
        // by taking the cross product between them and the chosen
        // basis vector.
        Direction unit_w = normalize(w);
        Direction a = (fabs(unit_w.x()) > 0.9) ? Direction(0, 1, 0) : Direction(1, 0, 0);
        Direction v = normalize(cross(unit_w, a));
        Direction u = cross(unit_w, v);

        m_axis[0] = u;
        m_axis[1] = v;
        m_axis[2] = unit_w;
    }

public:
    ONB(const Direction &w)
    {
        buildFromNormal(w);
    }

    Direction local(const Direction &v) const
    {
        return v.x() * m_axis[0] + v.y() * m_axis[1] + v.z() * m_axis[2];
    }

    Direction fromWorld(const Direction &v) const 
    {
        return Direction(dot(v, m_axis[0]), dot(v, m_axis[1]), dot(v, m_axis[2]));
    }

    Direction u() const { return m_axis[0]; }
    Direction v() const { return m_axis[1]; }
    Direction w() const { return m_axis[2]; }
};