#pragma once

// STD
#include <iostream>
#include <algorithm>
#include <fstream>
#include <memory>
#include <vector>
#include <list>
#include <string>
#include <cmath>
#include <thread>
#include <functional>

#include <debug.h>

#ifdef __GNUC__
#define PACKED( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACKED( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

const double inf = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

inline double degreeToRad(double degrees)
{
    return degrees / 180.0 * pi;
}

inline double clamp(double value, double min, double max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}