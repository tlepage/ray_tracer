#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cassert>
#include "../include/Bitmap.h"

Bitmap::Bitmap(uint32_t width, uint32_t height)
{
    image_data = {};
    image_data.width = width;
    image_data.height = height;

    output_pixel_size = get_total_pixel_size();
    image_data.pixels = (uint32_t *)malloc(output_pixel_size);
}

inline uint32_t Bitmap::get_total_pixel_size() const
{
    return (image_data.width * image_data.height * sizeof(uint32_t));
}

void Bitmap::write_image(const std::string &file_name)
{
    BitmapHeader header = compose_header();

    std::ofstream file (file_name, std::ios::out | std::ios::binary | std::ios::trunc);
    assert(file.is_open());
    file.write((char *)(&header), sizeof(header));
    file.write((char *)(image_data.pixels), output_pixel_size);
    file.close();
}

const ImageData *Bitmap::get_image_data() const
{
    return &image_data;
}

const BitmapHeader Bitmap::compose_header()
{
    BitmapHeader header = {};
    header.file_type = BITMAP_ID_FIELD;
    header.file_size = sizeof(header) + output_pixel_size;
    header.bitmap_offset = sizeof(header);
    header.size = sizeof(header) - HEADER_SIZE_EXCLUDE_BYTES;
    header.width = image_data.width;
    header.height = image_data.height;
    header.planes = BITMAP_PLANES;
    header.bits_per_pixel = BITS_PER_PIXEL;
    header.compression = 0;
    header.size_of_bitmap = output_pixel_size;
    header.horizontal_resolution = 0;
    header.vertical_resolution = 0;
    header.colors_used = 0;
    header.colors_important = 0;

    return header;
}
