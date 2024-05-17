#pragma once

#include <core.h>
#include <vec3.h>
#include <camera.h>
#include <ray.h>
#include <scene.h>

#include <color_array.h>

#include <hitables/hitable_list.h>
#include <bvh/bvh.h>

class Renderer
{
private:
    int m_width;
    int m_height;
    int m_samples_per_pixel = 200;
    int m_max_bounces = 12;
    int m_thread_amount = 16;

    Color m_background = Color(0);

    std::unique_ptr<ColorArray> m_screen_buf;

    BvhNode m_world;
    Scene m_scene;

private:
    Color rayColor(const Ray &r, int bounces);
    void renderThread(int thread_idx, double *percentage);
    void calcProgress(double *percentages);
    void generate_bvh();

public:
    Renderer() {}

    void set_threads(int threads);
    void set_samples_per_pixel(int samples);
    void set_max_bounces(int max_bounces);
    void set_dimensions(int width, int height);
    void set_background_color(Color bg);

    Scene& get_scene() { return m_scene; }
    int render();

    int writeToFile(std::string file);
};
