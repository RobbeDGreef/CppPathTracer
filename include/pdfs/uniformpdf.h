#pragma once

#include <pdfs/pdf.h>
#include <random.h>

class UniformPDF : public PDF
{
public:
    double value(const Direction &dir) const override
    {
        return 1 / (4 * pi);
    }

    Direction generate() const override
    {
        return randomGen.getPoint3InUnitSphere();
    }
};