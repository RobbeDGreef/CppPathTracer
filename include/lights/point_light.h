#pragma once

#include <vec3.h>
#include <lights/light.h>

class PointLight : public Light
{
private:
    Point3 m_position;

public:
    PointLight(const Color &color, double intensity, const Point3 &position) : Light(color, intensity), m_position(position) {}
    double getIntensityAtPoint(const Point3 &loc) const override;
    Direction getDirection(const Point3& loc) const override;
};