#pragma once

#include <vec3.h>
#include <color_array.h>
#include <textures/texture.h>
#include <bmp.h>

class ImageTexture : public Texture
{
private:
    std::shared_ptr<ColorArray> m_color_array;

public:
    ImageTexture(std::string image_file) : ImageTexture(Bmp::read(image_file)) {}
    ImageTexture(std::shared_ptr<ColorArray> colorarray) : m_color_array(colorarray) { WARN(colorarray->at(5)[5].to_string()); 
    DEBUG("width: " << m_color_array->width() << " height: " << m_color_array->height()); }

    Color value(double u, double v, const Point3 &p) const;
};