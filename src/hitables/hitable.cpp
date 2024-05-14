#include <hitables/hitable.h>
#include <random.h>

Point3 Hitable::randomPointIn() const
{
    AABB box;
    this->boundingBox(box);
    return randomGen.getPoint3() * (box.max() - box.min()) + box.min();
}

double Hitable::pdf(const Ray &r) const
{
    HitRecord rec;
    if (!this->hit(r, 0.001, inf, rec))
    {
        return 0;
    }

    double ds = rec.t * rec.t;
    double c = fabs(dot(r.direction(), rec.normal));
    AABB box;
    this->boundingBox(box);
    return ds / (c * (box.volume()));
}