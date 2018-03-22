#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cassert>
#include "../include/Bitmap.h"

Bitmap::Bitmap(uint32_t width, uint32_t height)
{
    ImageData image_data = {};
    image_data.width = width;
    image_data.height = height;

    output_pixel_size = calculate_total_pixel_size();
    image_data.pixels = std::shared_ptr<uint32_t>(new uint32_t[output_pixel_size],
                                                  std::default_delete<uint32_t[]>());
}

const inline uint32_t Bitmap::calculate_total_pixel_size() const
{
    return (image_data.width * image_data.height * sizeof(uint32_t));
}

void Bitmap::write_image(const std::string &file_name)
{
    BitmapHeader header = {};
    header.compose(image_data, output_pixel_size);

    std::ofstream file (file_name, std::ios::out | std::ios::binary | std::ios::trunc);
    assert(file.is_open());
    file.write(reinterpret_cast<char *>(&header), sizeof(header));
    file.write(reinterpret_cast<char *>(image_data.pixels.get()), output_pixel_size);
    file.close();
}

const ImageData *Bitmap::get_image_data() const
{
    return &image_data;
}
