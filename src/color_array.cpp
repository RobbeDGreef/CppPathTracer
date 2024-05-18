#include <color_array.h>

ColorArray::ColorArray(int width, int height)
{
    m_width = width;
    m_height = height;

    if (width == 0)
    {
        WARN("Invalid width given to ColorArray (0), setting to 1");
        m_width = 1;
    }
    if (height == 0)
    {
        WARN("Invalid height given to ColorArray (0), setting to 1");
        m_width = 1;
    }   

    m_array = new Color*[width];

    for (int i = 0; i < width; i++)
        m_array[i] = new Color[height]();
}

ColorArray::~ColorArray()
{
    if (m_array == nullptr) return;

    for (int i = 0; i < m_width; i++)
        delete[] m_array[i];
    
    delete[] m_array;
}
