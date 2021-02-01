#include <color_array.h>

ColorArray::ColorArray(int width, int height)
{
    m_width = width;
    m_height = height;

    m_array = new Color*[width];

    for (int i = 0; i < width; i++)
        m_array[i] = new Color[height];
}

ColorArray::~ColorArray()
{
    for (int i = 0; i < m_width; i++)
        delete[] m_array[i];
    
    delete[] m_array;
}
