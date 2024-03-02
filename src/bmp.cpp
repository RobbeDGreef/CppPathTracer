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
    header.size = 0; // This will be set at the end.
    
    header.image_data_offset = sizeof(struct bmp_hdr) +     // Header 1
                               sizeof(struct bmp_dib_hdr);  // Header 2
    
    output.write((const char *) &header, sizeof(struct bmp_hdr));

    // Write the second header
    struct bmp_dib_hdr dib;
    dib.size = BMP_DIB_SIZE;
    dib.image_width = width;
    dib.image_height = height;
    dib.planes_cnt = BMP_DIB_PLANES;
    dib.bpp = BMP_DIB_BBP;
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

        // Every row in the pixel array has to be padded to 4 bytes.
        if (width % 4)
            output.seekp(4 - (width % 4), std::ios::end);
    }

    int size = output.tellp();
    output.seekp(2, std::ios::beg);
    output.write((const char *) &size, 4);

    return 0;
}

#define READ_ERR(x) do { WARN(x); return std::make_shared<ColorArray>(0,0);} while (0);

std::shared_ptr<ColorArray> Bmp::read(std::string filename)
{
    std::ifstream input;
    input.open(filename, std::ios::binary | std::ios::in);

    if (!input.is_open())
        READ_ERR("Could not open '" << filename << "'");
    
    struct bmp_hdr header;
    input.read((char*) &header, sizeof(struct bmp_hdr));

    if (header.id[0] != BMP_HDR_ID_0 && header.id[1] != BMP_HDR_ID_1)
        READ_ERR("Could not read '" << filename << "' because it is not a valid BMP image");

    struct bmp_dib_hdr dib_header;
    input.read((char*) &dib_header, sizeof(struct bmp_dib_hdr));

    if (dib_header.compression_type != 0)
        READ_ERR("Could not read '" << filename << "' because the BMP image is compressed (" << dib_header.compression_type << ")");

    int width = dib_header.image_width;
    int height = dib_header.image_height;
    int bpp = dib_header.bpp / 8;
    auto array = std::make_shared<ColorArray>(width, height);

    input.seekg(header.image_data_offset, std::ios::beg);

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            uint8_t data[4];
            input.read((char*) data, bpp);
            array->at(i)[j] = Color((double) data[2] / 255, (double) data[1] / 255, (double) data[0] / 255);
        }

        // Every row in the pixel array has to be padded to be a multiple of 4 bytes. 
        // meaning we have to account for this gap.
        if (width % 4)
            input.seekg(4 - (width % 4), std::ios::cur);

    }

    return array;
}