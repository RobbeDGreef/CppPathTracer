#pragma once

#include <ray.h>
#include <vec3.h>
#include <hitables/hitable.h>
#include <pdfs/pdf.h>

struct ScatterRecord
{
    bool skip_pdf;
    Ray scattered_ray;
    std::shared_ptr<PDF> pdf;
};

class Material
{
public:
    virtual bool scatter(const Ray &r, const HitRecord &rec, ScatterRecord &srec) const = 0;
    virtual bool emitted(double u, double v, const Point3 &p, Color& emission) const { return false; }
    virtual Color eval(const Ray &in, const HitRecord &rec, const ScatterRecord &srec) const = 0;
};