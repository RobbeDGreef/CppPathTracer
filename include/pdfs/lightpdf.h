#pragma once

#include <pdfs/hitablepdf.h>

#define USE_AABB_FOR_LIGHT_SAMPLING 1

class LightPDF : public HitablePDF
{
public:

    LightPDF(const Point3& origin, const std::shared_ptr<HitableList>& lst) : HitablePDF(origin, lst) {}

    double value(const Direction &dir) const override
    {
        HitRecord rec;
        // TODO: this is very inefficient because we do double the ray intersects (once here over the hitlist and once in the pdf of each object)
        Ray r(m_origin, dir);
        if (!m_hitable_list->hit(r, 0.001, inf, rec))
        {
            // Return very small value that is not 0 because that would cause div's by zero 
            // and thus nans.
            return 0.00001;
        }

        double distance_squared = rec.t * rec.t;
        double cosine = fabs(dot(dir, rec.normal));

#if USE_AABB_FOR_LIGHT_SAMPLING
        AABB box;
        m_hitable_list->boundingBox(box);
        box.scale(0.90);

        //double pdf = 1.0 / box.volume();

        // I am really not sure about this *4 in the pdf, this is kinda a hack because there
        // is too little light falloff.
        double pdf = 1.0 / box.surfaceArea() * 4;
#else
        double pdf = m_hitable_list->pdf(r);
#endif

        return pdf * distance_squared;
    }
#if USE_AABB_FOR_LIGHT_SAMPLING
    Direction generate() const override
    {
        AABB box;
        m_hitable_list->boundingBox(box);
        box.scale(0.90);

        return box.randomPointIn() - m_origin;
    }
#endif

};