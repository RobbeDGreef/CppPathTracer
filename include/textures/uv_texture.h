#pragma once

#include <textures/texture.h>

class UVTexture : public Texture
{
public:
    UVTexture() {}
    Color value(double u, double v, const Point3 &p) const
    {
        return Color(u, v, 0.9);
    };
};