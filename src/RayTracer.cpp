#include <iostream>
#include "../include/Bitmap.h"
#include "../include/RayTracer.h"

#include "gtest/gtest.h"

void cast_rays(CastState *state)
{
    Scene *scene = state->scene;
    const float view_x = state->view_x;
    const float view_y = state->view_y;
    const float half_pixel_height = state->half_pixel_height;
    const float half_pixel_width = state->half_pixel_width;
    const Vector::Vector3 view_center = state->view_center;
    const float half_view_width = 0.5f * state->view_width;
    const float half_view_height = 0.5f * state->view_height;

    const Vector::Vector3 camera_x_axis = state->camera_x_axis;
    const Vector::Vector3 camera_y_axis = state->camera_y_axis;
    const Vector::Vector3 camera_position = state->camera_position;

    Math::RandomSeries series = state->series;

    uint64_t bounces_computed = 0;
    Vector::Vector3 final_color = {};

    for (uint32_t ray_index = 0; ray_index < RAYS_PER_PIXEL; ++ray_index)
    {
        float x_offset = view_x + random_bilateral(&series) * half_pixel_width;
        float y_offset = view_y + random_bilateral(&series) * half_pixel_height;

        Vector::Vector3 film_position = view_center + (x_offset * half_view_width * camera_x_axis)
                           + (y_offset * half_view_height * camera_y_axis);

        Vector::Vector3 ray_origin = camera_position;
        Vector::Vector3 ray_direction = Math::normalize_or_zero(film_position - camera_position);

        float min_hit_distance = MINIMUM_HIT_DISTANCE;
        float tolerance = TOLERANCE;

        Vector::Vector3 sample = {};
        auto attenuation = Vector::Vector3 {1, 1, 1};
        for (uint32_t bounces = 0; bounces < MAX_BOUNCE_COUNT; ++bounces)
        {
            Vector::Vector3 next_normal = {};
            float hit_distance = FLOAT32_MAX;

            MaterialName hit_material_name = MaterialName::White;
            ++bounces_computed;

            for (auto &plane : scene->planes)
            {
                float denominator = Math::inner_product(plane.normal, ray_direction);

                if ((denominator < -tolerance) || (denominator > tolerance))
                {
                    float t = (-plane.distance_from_origin - Math::inner_product(plane.normal, ray_origin)) / denominator;
                    if ((t > min_hit_distance) && (t < hit_distance))
                    {
                        hit_distance = t;
                        hit_material_name = plane.material_name;

                        next_normal = plane.normal;
                    }
                }
            }

            for (auto &sphere : scene->spheres)
            {
                Vector::Vector3 sphere_relative_ray_origin = ray_origin - sphere.position;
                float a = Math::inner_product(ray_direction, ray_direction);
                float b = 2.0f * Math::inner_product(ray_direction, sphere_relative_ray_origin);
                float c = Math::inner_product(sphere_relative_ray_origin, sphere_relative_ray_origin) - (sphere.radius * sphere.radius);
                float denominator = 2.0f * a;
                float root_term = Math::square_root(b * b - 4.0f * a * c);

                if (root_term > tolerance)
                {
                    float positive_term = (-b + root_term) / denominator;
                    float negative_term = (-b - root_term) / denominator;

                    float t = positive_term;
                    if ((negative_term > min_hit_distance) &&
                        (negative_term < positive_term)) // better hit (hit's in front of us and closer)
                    {
                        t = negative_term;
                    }
                    if ((t > min_hit_distance) && (t < hit_distance))
                    {
                        hit_distance = t;
                        hit_material_name = sphere.material_name;

                        next_normal = Math::normalize_or_zero(t * ray_direction + sphere_relative_ray_origin);
                    }
                }
            }

            if (hit_material_name != MaterialName::White)
            {
                Material material = MATERIALS.at(hit_material_name);

                sample += Math::hadamard_product(attenuation, material.emit_color);
                float cosine_attenuation = (Math::inner_product(-ray_direction, next_normal) + 0.5f);
                cosine_attenuation = std::max(cosine_attenuation, 0.0f);

                attenuation = Math::hadamard_product(attenuation, cosine_attenuation * material.reflection_color);
                ray_origin += hit_distance * ray_direction;
                Vector::Vector3 pure_bounce = ray_direction - 2.0f * Math::inner_product(ray_direction, next_normal) * next_normal;
                Vector::Vector3 random_bounce = Math::normalize_or_zero(next_normal +
                                                                Vector::Vector3 {random_bilateral(&series),
                                                                                 random_bilateral(&series),
                                                                                 random_bilateral(&series)});
                ray_direction = Math::normalize_or_zero(Math::lerp(random_bounce, material.specular, pure_bounce));
            }
            else
            {
                Material material = MATERIALS.at(MaterialName::White);
                sample += Math::hadamard_product(attenuation, material.emit_color);
                break;
            }
        }

        final_color += CONTRIBUTION * sample;
    }

    state->bounces_computed += bounces_computed;
    state->final_color = final_color;
}

auto get_pixel_pointer(ImageData image_data, uint32_t x, uint32_t y)
{
    uint32_t *result = image_data.pixels + x + y * image_data.width;
    return result;
}

auto synced_fetch_and_add(uint64_t volatile *value, uint64_t addend)
{
    uint64_t result = __sync_fetch_and_add(value, addend);
    return result;
}

bool render_tile(TileQueue *queue)
{
    uint64_t work_order_index = synced_fetch_and_add(&queue->next_tile_batch_index, 1);
    if (work_order_index >= queue->tile_batch_count)
    {
        return false;
    }
    TileBatch *order = queue->tile_batches + work_order_index;

    ImageData image_data = order->image_data;
    uint32_t x_min = order->x_min;
    uint32_t y_min = order->y_min;
    uint32_t one_past_x_max = order->one_past_x_max;
    uint32_t one_past_y_max = order->one_past_y_max;
    float film_distance = 1.0f;

    CastState state = {};

    state.scene = order->scene;
    state.series = order->entropy;

    state.camera_position = Vector::Vector3 {0, -10, 1};
    state.camera_z_axis = Math::normalize_or_zero(state.camera_position);
    state.camera_x_axis = Math::normalize_or_zero(Math::cross_product(Vector::Vector3 {0, 0, 1}, state.camera_z_axis));
    state.camera_y_axis = Math::normalize_or_zero(Math::cross_product(state.camera_z_axis, state.camera_x_axis));

    state.view_width = 1.0f;
    state.view_height = 1.0f;

    // correct ratio for unequal width and height
    if (image_data.width > image_data.height)
    {
        state.view_height = state.view_width * (static_cast<float>(image_data.height) / static_cast<float>(image_data.width));
    }
    else if (image_data.height > image_data.width)
    {
        state.view_width = state.view_height * (static_cast<float>(image_data.width) / static_cast<float>(image_data.height));
    }

    state.view_center = state.camera_position - (film_distance * state.camera_z_axis);

    state.half_pixel_width = 0.5f / image_data.width;
    state.half_pixel_height = 0.5f / image_data.height;

    state.bounces_computed = 0;

    for (uint32_t y = y_min; y < one_past_y_max; ++y)
    {
        uint32_t *pixels = get_pixel_pointer(image_data, x_min, y);
        state.view_y = -1.0f + 2.0f * (static_cast<float>(y) / static_cast<float>(image_data.height));
        for (uint32_t x = x_min; x < one_past_x_max; ++x)
        {
            state.view_x = -1.0f + 2.0f * (static_cast<float>(x) / static_cast<float>(image_data.width));

            cast_rays(&state);
            Vector::Vector3 final_color = state.final_color;

            Vector::Vector3 bitmap_color =
            {
                255.0f * Math::linear_to_sRGB(final_color.x),
                255.0f * Math::linear_to_sRGB(final_color.y),
                255.0f * Math::linear_to_sRGB(final_color.z)
            };

            uint32_t packed_bitmap_color_value = Math::pack_BGRA(bitmap_color);
            *pixels++ = packed_bitmap_color_value;
        }
    }

    synced_fetch_and_add(&queue->bounces_computed, state.bounces_computed);
    synced_fetch_and_add(&queue->tiles_done, 1);

    return true;
}

void *worker_thread(void *queue)
{
    auto *tile_queue = (TileQueue *)queue;
    while(render_tile(tile_queue)) {};

    return nullptr;
}

void create_thread(void *parameter)
{
    pthread_attr_t attr;
    pthread_t tid;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&tid, &attr, worker_thread, parameter);
    pthread_attr_destroy(&attr);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();

    std::cout << "clocks/sec: " << CLOCKS_PER_SEC << std::endl;

    Scene scene = {};
    scene.planes.push_back(Plane { Vector::Vector3 {0.0f, 0.0f, 1.0f}, 0.0f, MaterialName::Metallic });
    scene.spheres.push_back(Sphere { Vector::Vector3 {0.0f, 2.0f, 1.8f}, 0.5f, MaterialName::Orange});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-1.2f, 2.0f, 1.8f}, 0.5f, MaterialName::MirrorBlue});
    scene.spheres.push_back(Sphere { Vector::Vector3 {0.0f, 2.0f, 2.9f}, 0.5f, MaterialName::MirrorBlue});
    scene.spheres.push_back(Sphere { Vector::Vector3 {1.2f, 2.0f, 1.8f}, 0.5f, MaterialName::MirrorBlue});
    scene.spheres.push_back(Sphere { Vector::Vector3 {0.0f, 2.0f, 0.7f}, 0.5f, MaterialName::MirrorBlue});

    scene.spheres.push_back(Sphere { Vector::Vector3 {0.8f, -3.6f, 0.3f}, 0.25f, MaterialName::Green});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-1.7f, 4.2f, 0.3f}, 0.1f, MaterialName::LightBlue});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-2.0f, 3.6f, 0.3f}, 0.1f, MaterialName::LightBlue});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-2.5f, 3.2f, 0.3f}, 0.1f, MaterialName::LightBlue});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-3.0f, 2.8f, 0.3f}, 0.1f, MaterialName::LightBlue});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-3.4f, 2.4f, 0.3f}, 0.1f, MaterialName::LightBlue});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-4.0f, 2.6f, 0.3f}, 0.1f, MaterialName::LightBlue});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-4.5f, 2.8f, 0.3f}, 0.1f, MaterialName::LightBlue});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-5.0f, 3.2f, 0.3f}, 0.1f, MaterialName::LightBlue});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-5.5f, 3.6f, 0.3f}, 0.1f, MaterialName::LightBlue});

    scene.spheres.push_back(Sphere { Vector::Vector3 {-1.2f, -4.6f, 0.3f}, 0.1f, MaterialName::Raspberry});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-1.8f, -4.6f, 0.3f}, 0.1f, MaterialName::Raspberry});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-1.4f, -5.3f, 0.3f}, 0.1f, MaterialName::Raspberry});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-1.6f, -4.0f, 0.3f}, 0.1f, MaterialName::Raspberry});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-1.4f, -5.0f, 0.15f}, 0.1f, MaterialName::Green});

    scene.spheres.push_back(Sphere { Vector::Vector3 {4.0f, 1.0f, 2.0f}, 1.5f, MaterialName::Violet});
    scene.spheres.push_back(Sphere { Vector::Vector3 {-4.0f, 5.0f, 1.0f}, 2.0f, MaterialName::LightGreen});

    scene.spheres.push_back(Sphere { Vector::Vector3 {7.0f, 17.0f, 0.0f}, 5.0f, MaterialName::LightBlueReflective});

    Bitmap bitmap = Bitmap(IMAGE_WIDTH, IMAGE_HEIGHT);
    std::unique_ptr<ImageData> image_data = bitmap.get_image_data();

    // 64x64 tiles seem to be a sweet spot; keeping at that resolution
    uint32_t tile_width = 64;  // image_data->width / CORE_COUNT;
    uint32_t tile_height = 64; // tile_width;

    uint32_t tile_count_x = (IMAGE_WIDTH + tile_width - 1) / tile_width;
    uint32_t tile_count_y = (IMAGE_HEIGHT + tile_height - 1) / tile_height;
    uint32_t total_tiles = tile_count_x * tile_count_y;

    std::cout << "Total tiles " << total_tiles << std::endl;
    TileQueue queue = {};
    queue.tile_batches = reinterpret_cast<TileBatch *>(malloc(total_tiles * sizeof(TileBatch)));

    std::cout << "Configuration: " << CORE_COUNT << " cores with " << tile_width << "x" << tile_height
              << " (" << (tile_width * tile_height * sizeof(uint32_t) / 1024) << "k/tile) " << "tiles\n";
    std::cout << "Quality: " << RAYS_PER_PIXEL << " rays/pixel, " << MAX_BOUNCE_COUNT << " bounces (max) per ray\n";

    for (uint32_t tile_y = 0; tile_y < tile_count_y; ++tile_y)
    {
        uint32_t min_y = tile_y * tile_height;
        uint32_t one_past_max_y = min_y + tile_height;
        if (one_past_max_y > IMAGE_HEIGHT)
        {
            one_past_max_y = IMAGE_HEIGHT;
        }

        for (uint32_t tile_x = 0; tile_x < tile_count_x; ++tile_x)
        {
            uint32_t min_x = tile_x * tile_width;
            uint32_t one_past_max_x = min_x + tile_width;

            if (one_past_max_x > IMAGE_WIDTH)
            {
                one_past_max_x = IMAGE_WIDTH;
            }

            TileBatch *batch = queue.tile_batches + queue.tile_batch_count++;
            assert(queue.tile_batch_count <= total_tiles);

            batch->scene = &scene;
            batch->image_data = *image_data;
            batch->x_min = min_x;
            batch->y_min = min_y;
            batch->one_past_x_max = one_past_max_x;
            batch->one_past_y_max = one_past_max_y;
            batch->entropy = { tile_x * 13843 + tile_y * 24892 };  // need actual source of entropy
        }
    }
    assert(queue.tile_batch_count == total_tiles);

    // this synced fetch is strictly for fencing
    // core 0 is not obligated to flush results yet
    // complete all writes we're going to do before all the other threads are created
    // to make sure all tile batches got filled
    // not *entirely* necessary, but it doesn't hurt
    synced_fetch_and_add(&queue.next_tile_batch_index, 0);

    clock_t start_clock = clock();

    // To turn on/off multi-threading
#if 1
    // core zero is occupied by main thread
    for (uint32_t core_index = 1; core_index < CORE_COUNT; ++core_index)
    {
        create_thread(&queue);
    }
#endif

    while (queue.tiles_done < total_tiles)
    {
        if (render_tile(&queue))
        {
            std::cout << "\rRay casting " << (100 * static_cast<uint32_t>(queue.tiles_done) / total_tiles) << "%...";
            fflush(stdout);
        }
    }
    clock_t end_clock = clock();

    double time_elapsed = (end_clock - start_clock) / ((CLOCKS_PER_SEC * CORE_COUNT) / 1000);
    std::cout << std::endl;
    std::cout << "Ray casting time: " << time_elapsed << "ms\n";
    std::cout << "Total bounces: " << queue.bounces_computed << std::endl;
    std::cout << "Performance: " << std::fixed << (time_elapsed / queue.bounces_computed) << "ms/bounce\n";
    bitmap.write_image("test.bmp");

    std::cout << "\nShit's Done, Bitch!\n";
    return 0;
}
