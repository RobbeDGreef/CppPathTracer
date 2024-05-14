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
    int m_samples_per_pixel;
    int m_max_bounces;
    int m_thread_amount = 16;

    Color m_background;

    ColorArray m_screen_buf;

    BvhNode m_world;
    Scene m_scene;

private:
    Color rayColor(const Ray& r, int bounces);
    void renderThread(int thread_idx, double *percentage);
    void calcProgress(double *percentages);

public:
    Renderer(int width, int height, Scene scene, Color bg=Color());
    int render(int samples=100, int max_bounces=12);

    int writeToFile(std::string file);
};