#pragma once

#include <vec3.h>

class PDF
{
public:
    virtual double value(const Direction& d) const = 0;
    virtual Direction generate() const = 0;
};