#pragma once

#include <pdfs/pdf.h>
#include <materials/helpers.h>
#include <random.h>

class FresnelPDF : public PDF
{
private:
    double m_mix;
    Direction m_view;
    std::shared_ptr<PDF> m_p1;
    std::shared_ptr<PDF> m_p2;

public:
    FresnelPDF(double mix, Direction view, std::shared_ptr<PDF> p1, std::shared_ptr<PDF> p2)
        : m_mix(mix), m_view(view), m_p1(p1), m_p2(p2) {}

    double value(const Direction &dir) const override
    {
        double fr = schlickFresnel(0.04, dot(m_view, dir));
        return lerp(m_p1->value(dir), m_p2->value(dir), fr);
    }

    Direction generate() const override
    {
        if (randomGen.getDouble() < m_mix)
        {
            return m_p1->generate();
        }
        else
        {
            return m_p2->generate();
        }
    }
};