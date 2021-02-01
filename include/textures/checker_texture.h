#pragma once

#include <vec3.h>
#include <textures/texture.h>

class CheckerTexture : public Texture
{
private:
    Color m_color_even;
    Color m_color_odd;
    double m_scale;

public:
    CheckerTexture(Color color_even=Color(0,0,0), Color color_odd=Color(1,1,1), double scale=10) 
                   : m_color_even(color_even), m_color_odd(color_odd), m_scale(scale) {}

    Color value(double u, double v, const Point3 &p) const override
    {
        #if 0
        if (sin(m_scale * p.x()) * sin(m_scale * p.y()) * sin(m_scale * p.z()) > 0)
            return m_color_even;
        else 
            return m_color_odd;
        #endif
        #if 1
        if ((static_cast<int>(u * m_scale) % 2 && !(static_cast<int>(v * m_scale) % 2)) || 
            (static_cast<int>(v * m_scale) % 2 && !(static_cast<int>(u * m_scale) % 2)))
            return m_color_even;
        else 
            return m_color_odd;
        #endif
    }
};