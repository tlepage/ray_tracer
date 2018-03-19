#pragma once
#include <cstdint>
#include <memory>

constexpr uint32_t BITMAP_ID_FIELD = 0x4D42;

// this is BitmapOffset(4) - FileSize(4) - FileType(2) - Reserved1(2) - Reserved(2) in Bytes
constexpr uint32_t HEADER_SIZE_EXCLUDE_BYTES = 14;
constexpr uint32_t BITMAP_PLANES = 1;
constexpr uint32_t BITS_PER_PIXEL = 32;

struct ImageData
{
    uint32_t width;
    uint32_t height;
    uint32_t *pixels;
};

#pragma pack(push, 1)
struct BitmapHeader
{
    uint16_t file_type;
    uint32_t file_size;
    uint16_t reserved_1;
    uint16_t reserved_2;
    uint32_t bitmap_offset;
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t size_of_bitmap;
    uint32_t horizontal_resolution;
    uint32_t vertical_resolution;
    uint32_t colors_used;
    uint32_t colors_important;

    void compose(ImageData image_data, uint32_t output_pixel_size)
    {
        file_type = BITMAP_ID_FIELD;
        file_size = sizeof(BitmapHeader) + output_pixel_size;
        bitmap_offset = sizeof(BitmapHeader);
        size = sizeof(BitmapHeader) - HEADER_SIZE_EXCLUDE_BYTES;
        width = image_data.width;
        height = image_data.height;
        planes = BITMAP_PLANES;
        bits_per_pixel = BITS_PER_PIXEL;
        compression = 0;
        size_of_bitmap = output_pixel_size;
        horizontal_resolution = 0;
        vertical_resolution = 0;
        colors_used = 0;
        colors_important = 0;
    };
};
#pragma pack(pop)

class Bitmap
{
private:
    ImageData image_data;
    uint32_t output_pixel_size;

    const uint32_t calculate_total_pixel_size() const;
public:
    Bitmap(uint32_t width, uint32_t height);
    void write_image(const std::string &file_name);
    const ImageData *get_image_data() const;
};
