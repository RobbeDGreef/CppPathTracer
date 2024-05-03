#pragma once

class Light 
{
protected:
    Color m_color;
    double m_intensity;

public:
    Light(const Color &color, double intensity) : m_color(color), m_intensity(intensity) {}
    virtual double getIntensityAtPoint(const Point3& loc) const { return 1.0; };
    virtual double getDistanceTo(const Point3& loc) const { return inf; }
    virtual Direction getDirection(const Point3& loc) const { return Direction(0,0,0); };
};