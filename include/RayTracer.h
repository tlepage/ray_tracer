#pragma once
#include <cfloat>
#include <array>
#include <map>
#include <cmath>
#include <algorithm>
#include <vector>
#include "Math.h"

constexpr float FLOAT32_MAX = FLT_MAX;
constexpr float MINIMUM_HIT_DISTANCE = 0.001f;
constexpr float TOLERANCE = 0.0001f;
constexpr uint32_t IMAGE_WIDTH = 1280;
constexpr uint32_t IMAGE_HEIGHT = 720;
constexpr uint32_t CORE_COUNT = 8;
constexpr uint32_t MAX_BOUNCE_COUNT = 8;
constexpr uint32_t RAYS_PER_PIXEL = 512;
constexpr float CONTRIBUTION = (1.0f / (static_cast<float>(RAYS_PER_PIXEL)));

enum class MaterialName
{
    White,
    Metallic,
    Orange,
    Violet,
    LightGreen,
    Green,
    MirrorBlue,
    LightBlue,
    Raspberry,
    LightBlueReflective
};

struct Material
{
    float specular; // 0 is pure diffuse, 1 is pure specular
    Math::Vector3 emit_color;
    Math::Vector3 reflection_color;
};

// TODO(tom): make consts out of the color names
const std::map<const MaterialName, const Material> MATERIALS =
{
    { MaterialName::White, Material {0.5f, Math::Vector3 {1.0f, 1.0f, 1.0f}, Math::Vector3 {} }},
    { MaterialName::Metallic, Material {0.8f, Math::Vector3 {}, Math::Vector3 {0.5f, 0.5f, 0.5f} }},
    { MaterialName::Orange, Material {0.1f, Math::Vector3 {3.0f, 0.0f, 0.0f}, Math::Vector3 {1.0f, 0.31f, 0.098f} }},
    { MaterialName::Violet, Material {0.6f, Math::Vector3 {}, Math::Vector3 {1.0f, 0.1f, 0.9f} }},
    { MaterialName::LightGreen, Material {0.7f, Math::Vector3 {}, Math::Vector3 {0.65f, 1.0f, 0.1f} }},
    { MaterialName::Green, Material {0.8f, Math::Vector3 {0.1f, 1.0f, 0.02f}, Math::Vector3 {0.1f, 1.0f, 0.02f} }},
    { MaterialName::MirrorBlue, Material {1.0f, Math::Vector3 {}, Math::Vector3 {0.0f, 0.25f, 1.0f} }},
    { MaterialName::LightBlue, Material {0.8f, Math::Vector3 {0.01f, 1.0f, 0.9f}, Math::Vector3 {0.01f, 1.0f, 0.9f} }},
    { MaterialName::Raspberry, Material {0.9f, Math::Vector3 {}, Math::Vector3 {1.0f, 0.01f, 0.49f} }},
    { MaterialName::LightBlueReflective, Material {0.98f, Math::Vector3 {}, Math::Vector3 {0.01f, 1.0f, 0.9f} }}
};

struct Sphere
{
    Math::Vector3 position;
    float radius;
    MaterialName material_name;
};

struct Plane
{
    Math::Vector3 normal;
    float distance_from_origin;
    MaterialName material_name;
};

struct Scene
{
    std::vector<Plane> planes;
    std::vector<Sphere> spheres;
};

struct CastState
{
    Scene *scene;
    float view_x;
    float view_y;
    float view_width;
    float view_height;
    float half_pixel_width;
    float half_pixel_height;
    Math::Vector3 view_center;
    Math::Vector3 camera_x_axis;
    Math::Vector3 camera_y_axis;
    Math::Vector3 camera_z_axis;
    Math::Vector3 camera_position;
    Math::RandomSeries series;

    Math::Vector3 final_color;
    uint64_t bounces_computed;
};

struct TileBatch
{
    Scene *scene;
    ImageData image_data;
    uint32_t x_min;
    uint32_t y_min;
    uint32_t one_past_x_max;
    uint32_t one_past_y_max;
    Math::RandomSeries entropy;
};

struct TileQueue
{
    uint32_t tile_batch_count;
    TileBatch *tile_batches;

    volatile uint64_t next_tile_batch_index;
    volatile uint64_t bounces_computed;
    volatile uint64_t tiles_done;
};

