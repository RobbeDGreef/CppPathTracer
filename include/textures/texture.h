#pragma once

#include <vec3.h>

class Texture
{
public:
    virtual Color value(double u, double v, const Point3 &p) const = 0;
};

class SolidColor : public Texture
{
private:
    Color m_color;
public:
    SolidColor() {}
    SolidColor(Color c) : m_color(c) {}

    Color value(double u, double v, const Point3 &p) const override { return m_color; }
};