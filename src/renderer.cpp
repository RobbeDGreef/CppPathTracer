#include <renderer.h>
#include <bmp.h>
#include <hitables/hitable.h>
#include <random.h>
#include <materials/material.h>
#include <chrono>
#include <core.h>
#include <scene.h>

#define RAY_NEAR_CLIP 0.001
#define RAY_FAR_CLIP inf

Renderer::Renderer(int width, int height, Scene scene, Color bg)
    : m_screen_buf(width, height),
      m_world(scene.getHitableList()), m_scene(scene)
{
    m_width = width;
    m_height = height;
    m_background = bg;
}

static Point3 randomInHemisphere(const Direction normal)
{
    Point3 in_unit_sphere = randomInUnitSphere();
    if (dot(in_unit_sphere, normal) > 0.0)
        return in_unit_sphere;
    else
        return -in_unit_sphere;
}

Color Renderer::rayColor(const Ray &r, int bounces = 0)
{
    if (bounces == m_max_bounces)
        return Color(0, 0, 0);

    // If we do not hit anything with this ray, we return the background color / texture
    HitRecord rec;
    if (!m_world.hit(r, RAY_NEAR_CLIP, RAY_FAR_CLIP, rec))
    {
        // TODO: implement HDRI
        return m_background;
    }

    // We did hit an object, now we calculate its color based on its material, the lights in the scene etc
    Ray scattered;
    Color attenuation;
    
    // The emitted function returns the amount of emission the material has, 
    // if this is none, we assume the output variable is not changed and thus stays 0
    Color output = Color(0);
    rec.mat->emitted(rec.u, rec.v, rec.p, output);

#if 0
    // do ray hit with far clip being the distance to the light i think
    for (Light& light : m_scene.getLightList()) {
        
        int hits = 0;
        for (int i = 0; i < m_shadow_ray_count; i++) {
            HitRecord shadow_rec;
            const Ray r = Ray(r.origin(), -light.getDirection(r.origin()));
            m_world.hit(r, 0, light.getDistanceTo(r.origin()), shadow_rec);
        }
    }
#endif

    // Scatter returns wether the material can scatter and the attentuation the material has.
    // This is essentially one smaple in the intergral of the rendering equation.
    Color nextRayColor = Color(1);
    if (rec.mat->scatter(r, rec, attenuation, scattered))
    {
        nextRayColor = rayColor(scattered, bounces + 1);
    }

    // TODO: is this correct, should you return the attenuation if you stop scattering?
    // TODO: should emissive material output (already in output variable) be multiplied with attenuation or not

    output += attenuation * nextRayColor;

    return output;
}

void Renderer::calcProgress(double *percentages)
{
    double cnt = 0;
    while (cnt < 1.0)
    {
        cnt = 0;
        for (int i = 0; i < m_thread_amount; i++)
        {
            cnt += percentages[i];
        }

        cnt /= m_thread_amount;

        OUT(cnt * 100 << "% completed");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void Renderer::renderThread(int thread_idx, double *percentages)
{
    int work = m_height / m_thread_amount;
    int extra = 0;

    if (thread_idx == m_thread_amount - 1)
        extra = (m_height - work * m_thread_amount);

    for (int j = work * thread_idx; j < work * thread_idx + work + extra; ++j)
    {
        percentages[thread_idx] = (double)(j - work * thread_idx) / (work + extra);

        for (int i = 0; i < m_width; ++i)
        {
            Color pixel_color;
            for (int s = 0; s < m_samples_per_pixel; ++s)
            {
                double x = ((double)i + randomDouble()) / (m_width - 1);
                double y = ((double)j + randomDouble()) / (m_height - 1);
                Ray r = m_scene.getCamera().sendRay(x, y);
                pixel_color += rayColor(r);
            }
            m_screen_buf[i][j] = clamp((sqrt(pixel_color / m_samples_per_pixel)), 0.0, 0.999);
        }
    }
    OUT("thread " << thread_idx << " has finished");
}

int Renderer::render(int samples, int bounces)
{
    m_samples_per_pixel = samples;
    m_max_bounces = bounces;
    auto start_chrono = std::chrono::high_resolution_clock::now();

    OUT("Rendering on " << m_thread_amount << " threads");

    double *percentages = new double[m_thread_amount];

    // This multithreading approach is not great, we divide the work in
    // 'm_thread_amount' parts and this means that some threads will finish
    // before others. Somtimes even at 20%. This means that those threads are not
    // being used for the rest of the render, which is wastefull, a better solution
    // would be to queue the work in smaller parts and feed the threads like that
    // but this will do for now.
    std::thread *threads = new std::thread[m_thread_amount];
    for (int i = 0; i < m_thread_amount; i++)
    {
        threads[i] = std::thread(&Renderer::renderThread, this, i, percentages);
    }

    // Display the progress
    std::thread(&Renderer::calcProgress, this, percentages).detach();

    for (int i = 0; i < m_thread_amount; i++)
    {
        threads[i].join();
    }

    auto stop_chrono = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_chrono - start_chrono);
    DEBUG("Rendering done, took: " << (double)duration.count() / 1000 << " seconds");

    delete[] percentages;
    delete[] threads;
    return 0;
}

int Renderer::writeToFile(std::string file)
{
    return Bmp::write(m_screen_buf, file, m_width, m_height);
}