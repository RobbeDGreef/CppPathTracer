#include <bmp.h>

// This code is a little ugly and pretty c style ish but 
// it will do for now.

int Bmp::write(ColorArray &color_array, std::string filename, int width, int height)
{
    std::ofstream output;
    output.open(filename, std::ios::out| std::ios::binary);

    if (!output.is_open())
        return -1;

    // Write the first header
    struct bmp_hdr header;
    header.id[0] = BMP_HDR_ID_0;
    header.id[1] = BMP_HDR_ID_1;
    header.size = sizeof(struct bmp_hdr) +                  // Header 1
                  sizeof(struct bmp_dib_hdr) +              // Header 2
                  width * height * (BMP_DIB_BBP / 8);       // The pixel data
    
    DEBUG("size: " << header.size);
    
    header.image_data_offset = sizeof(struct bmp_hdr) +     // Header 1
                               sizeof(struct bmp_dib_hdr);  // Header 2
    
    output.write((const char *) &header, sizeof(struct bmp_hdr));

    // Write the second header
    struct bmp_dib_hdr dib;
    dib.size = BMP_DIB_SIZE;
    dib.image_width = width;
    dib.image_height = height;
    dib.planes_cnt = BMP_DIB_PLANES;
    dib.bbp = BMP_DIB_BBP;
    dib.compression_type = BMP_DIB_COMPRESSION_TYPE;
    dib.compressed_size = 0;
    dib.x_ppm = 0;
    dib.y_ppm = 0;
    dib.used_colors_cnt = 0;
    dib.important_colors_cnt = 0;

    output.write((const char *) &dib, sizeof(struct bmp_dib_hdr));
    
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            Color &color = color_array[i][j];
            output.put(static_cast<char>(255.999 * color[2]));
            output.put(static_cast<char>(255.999 * color[1]));
            output.put(static_cast<char>(255.999 * color[0]));
        }
    }

    return 0;
}