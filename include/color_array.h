#pragma once

#include <vec3.h>

class ColorArray
{
private:
    int m_width = 0;
    int m_height = 0;

    Color **m_array = nullptr;

public:
    ColorArray() {}
    ColorArray(int width, int height);
    ~ColorArray();

    int width() const { return m_width; }
    int height() const { return m_height; }

    Color *operator[](int i) { return m_array[i]; }
    Color *at(int i) { return m_array[i]; }
};
