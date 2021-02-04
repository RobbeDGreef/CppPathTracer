#include <hitables/sphere.h>

class MovingSphere : public Sphere
{
protected:
    Point3 center(double t) const override 
    {
        return m_center + Point3(0,0.5*t, 0);
    }

public:
    MovingSphere(Point3 center, double radius, std::shared_ptr<Material> mat)
                 : Sphere(center, radius, mat) {}
};
