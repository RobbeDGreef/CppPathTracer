#include <lights/point_light.h>

double PointLight::getIntensityAtPoint(const Point3& loc) const {
    // According to the inverse square law, light intensity decreased depending on the distance from the light
    Point3 d = loc - m_position;
    return m_intensity / d.length_squared();
}

Direction PointLight::getDirection(const Point3& loc) const {
    return normalize(m_position - loc);
}