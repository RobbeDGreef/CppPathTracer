#pragma once

#include <pdfs/pdf.h>
#include <random.h>
#include <onb.h>

class CosinePDF : public PDF
{
private:
    ONB m_base;

public:
    CosinePDF(const Direction &w) : m_base(w) {}

    double value(const Direction &dir) const override
    {
        double theta = dot(dir, m_base.w());
        return fmax(theta / pi, 0);
    }

    Direction generate() const override
    {
        double r1 = randomGen.getDouble();
        double r2 = randomGen.getDouble();

        double phi = 2 * pi * r1;
        Direction dir = Direction(cos(phi) * sqrt(r2), sin(phi) * sqrt(r2), sqrt(1 - r2));

        return m_base.local(dir);
    }
};