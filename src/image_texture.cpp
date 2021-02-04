#include <textures/image_texture.h>

Color ImageTexture::value(double u, double v, const Point3 &p) const
{
    int i = static_cast<int>(u * m_color_array->width());
    int j = static_cast<int>(v * m_color_array->height());

    if (u > 1 ||v > 1)
    {
        WARN("Trying to get UV value out of texture range");
        return Color();
    }
    
    return m_color_array->at(i)[j];
}