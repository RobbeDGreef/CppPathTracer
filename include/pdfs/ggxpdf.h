#pragma once

#include <pdfs/pdf.h>
#include <random.h>
#include <hitables/hitable.h>
#include <hitables/hitable_list.h>
#include <onb.h>
#include <materials/helpers.h>

class GGXPDF : public PDF
{
protected:
    Ray m_view;
    Direction m_normal;
    double m_roughness;

public:
    GGXPDF(const Ray &r, const Direction &normal, double roughness) : m_view(r), m_normal(normal), m_roughness(roughness) {}

    double value(const Direction &dir) const override
    {
        Direction v = normalize(-m_view.direction());
        Direction n = normalize(m_normal);
        Direction h = normalize(v + dir);

        double ndotv = dot(n, v);
        double ndoth = dot(n, h);
        double vdoth = dot(v, h);

        return fmax(distributionGGX(n, h, m_roughness) * ndoth / (4 * vdoth), 0);
    }

    Direction generate() const override
    {
        ONB base(m_normal);
        Direction v = normalize(-m_view.direction());

        // TODO: see https://schuttejoe.github.io/post/ggximportancesamplingpart2/ for better sampler
        double r0 = randomGen.getDouble();
        double r1 = randomGen.getDouble();
        double a2 = m_roughness * m_roughness;
        double theta = acos(sqrtf((1 - r0) / ((a2 - 1) * r0 + 1)));
        double phi = 2 * pi * r1;
        // H is the microfacet normal
        Direction h = sphericalToCartesian(theta, phi);
        Direction world_h = base.local(h);

        Direction out = world_h * 2 * dot(world_h, v) - v;
        return out;
    }
};