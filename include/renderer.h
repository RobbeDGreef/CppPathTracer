#pragma once

#include <core.h>
#include <vec3.h>
#include <camera.h>
#include <ray.h>
#include <scene.h>
#include <color_array.h>

#include <hitables/hitable_list.h>
#include <bvh/bvh.h>

#include <queue>
#include <optional>

struct RenderWorkBlock
{
    int x;
    int y;
    int x_end;
    int y_end;
};

class RenderWorkQueue
{
private:
    std::queue<RenderWorkBlock> m_queue;
    std::mutex m_mutex;

public:
    RenderWorkQueue(std::queue<RenderWorkBlock> &work) : m_queue(work) {}
    std::optional<RenderWorkBlock> pop()
    {
        m_mutex.lock();

        if (m_queue.size() == 0)
        {
            m_mutex.unlock();
            return {};
        }

        RenderWorkBlock block = m_queue.front();
        m_queue.pop();

        m_mutex.unlock();
        return block;
    }

    int size()
    {
        // I dont think this has to be locked?
        return m_queue.size();
    }
};

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

    BvhManager m_world;
    Scene m_scene;

private:
    Color rayColor(const Ray &r, int bounce, int x, int y, int sample);

    void generate_bvh();

    void renderPixel(ColorArray* array, int x, int y);
#if THREADING_IMPLEMENTATION == THREAD_IMPL_NAIVE
    void renderThread(ColorArray* buffer, int thread_idx, double *percentage);
    void calcProgress(double *percentages);
#endif
#if THREADING_IMPLEMENTATION == THREAD_IMPL_OPENMP_BLOCKS
    void renderBlock(ColorArray* buffer, RenderWorkBlock work);
#endif

public:
    Renderer() {}

    void set_threads(int threads);
    void set_samples_per_pixel(int samples);
    void set_max_bounces(int max_bounces);
    void set_dimensions(int width, int height);
    void set_background_color(Color bg);

    Scene &get_scene() { return m_scene; }

    int render();

    int writeToFile(std::string file);
};
