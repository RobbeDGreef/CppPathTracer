#pragma once

#include <core.h>
#include <vec3.h>

#include <stdint.h>

#define BMP_HDR_ID_0 'B'
#define BMP_HDR_ID_1 'M'

#define BMP_DIB_SIZE                40
#define BMP_DIB_PLANES              1
#define BMP_DIB_BBP                 24  // We use 24 bits per pixel RGB
#define BMP_DIB_COMPRESSION_TYPE    0
#define BMP_DIB_COMPRESSION_SIZE    0

PACKED(
struct bmp_hdr
{
    char        id[2];
    uint32_t    size;                   // Size of the file in bytes
    uint16_t    reserved[2];
    uint32_t    image_data_offset;
});

PACKED(
struct bmp_dib_hdr
{
    uint32_t    size;                   // dib hdr size (should be 40)
    uint32_t    image_width;
    uint32_t    image_height;
    uint16_t    planes_cnt;             // Should be 1
    uint16_t    bbp;                    // Bits per pixel
    uint32_t    compression_type;       // 0 for no compression
    uint32_t    compressed_size;        // 0 when no compression 
    uint32_t    x_ppm;                  // Horizontal resolution (pixels per meter)
    uint32_t    y_ppm;                  // Vertical resolution (pixels per meter)
    uint32_t    used_colors_cnt;        // Amount of used colors
    uint32_t    important_colors_cnt;   // Amount of important colors;
});

int createBitmap(Color **color_array, std::string filename, int width, int height);