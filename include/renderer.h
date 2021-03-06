#pragma once

#include <core.h>
#include <vec3.h>
#include <camera.h>
#include <ray.h>

#include <color_array.h>

#include <hitables/hitable_list.h>
#include <bvh/bvh.h>

class Renderer
{
private:
    int m_width;
    int m_height;
    int m_samples_per_pixel;
    int m_max_bounces;
    int m_thread_amount = 8;

    Color m_background;

    ColorArray m_screen_buf;

    Camera m_cam;
    BvhNode m_world;

private:
    Color rayColor(const Ray& r, int bounces);
    void renderThread(int thread_idx, double *percentage);
    void calcProgress(double *percentages);

public:
    Renderer(int width, int height, HitableList world, Camera cam, Color bg=Color());
    int render(int samples=100, int max_bounces=12);

    int writeToFile(std::string file);
};