#pragma once

#include <pdfs/pdf.h>
#include <random.h>

class MixturePDF : public PDF
{
private:
    double m_mix;
    std::shared_ptr<PDF> m_p1;
    std::shared_ptr<PDF> m_p2;

public:
    MixturePDF(double mix, std::shared_ptr<PDF> p1, std::shared_ptr<PDF> p2)
        : m_mix(mix), m_p1(p1), m_p2(p2) {}

    double value(const Direction &dir) const override
    {
        return m_mix * m_p1->value(dir) + (1.0 - m_mix) * m_p2->value(dir);
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