#pragma once

#include <pdfs/pdf.h>
#include <random.h>
#include <hitables/hitable.h>
#include <hitables/hitable_list.h>

class HitablePDF : public PDF
{
protected:
    Point3 m_origin;
    std::shared_ptr<HitableList> m_hitable_list;

public:
    HitablePDF(const Point3& origin, const std::shared_ptr<HitableList>& hitables) : m_origin(origin), m_hitable_list(hitables) {}

    double value(const Direction &dir) const override
    {
        HitRecord rec;
        Ray r(m_origin, dir);
        return m_hitable_list->pdf(r);
    }

    Direction generate() const override
    {
        Point3 r = m_hitable_list->randomPointIn();
        //DEBUG("Random " << r.to_string() << " origin " << m_origin.to_string());
        return r - m_origin;
    }
};