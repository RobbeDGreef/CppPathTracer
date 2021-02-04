#include <renderer.h>
#include <bmp.h>
#include <hitable/hitable.h>
#include <random.h>
#include <materials/material.h>
#include <chrono>
#include <core.h>

Renderer::Renderer(int width, int height, HitableList world, Camera cam, Color bg)
                  : m_cam(cam), m_screen_buf(width, height),
                    m_world(world, cam.shutterStart(), cam.shutterEnd())
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

Color Renderer::rayColor(const Ray &r, int bounces=0)
{
    if (bounces == m_max_bounces)
        return Color(0,0,0);

    HitRecord rec;
    if (m_world.hit(r, 0.001, inf, rec))
    {
        Ray scattered;
        Color attenuation;
        Color emitted = rec.mat->emitted(rec.u, rec.v, rec.p);
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return emitted + attenuation * rayColor(scattered, bounces+1);
        
        return emitted;
    }
    
    return m_background;
}

void Renderer::calcProgress(double *percentages)
{
    double cnt = 0;
    while (cnt != 1.0)
    {
        cnt = 0;
        for (int i = 0; i < m_thread_amount; i++)
        {
            cnt += percentages[i];
        }

        cnt /= m_thread_amount;

        DEBUG(cnt * 100 << "% completed");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    delete[] percentages;
}

void Renderer::renderThread(int thread_idx, double *percentages)
{
    int work = m_height / m_thread_amount;
    int extra = 0;

    if (thread_idx == m_thread_amount - 1)
        extra = (m_height - work * m_thread_amount);
    
    for (int j = work * thread_idx; j < work * thread_idx + work + extra; ++j)
    {
        percentages[thread_idx] = (double) (j - work * thread_idx) / (work + extra);
        
        for (int i = 0; i < m_width; ++i)
        {
            Color pixel_color;
            for (int s = 0; s < m_samples_per_pixel; ++s)
            {
                double x = ((double) i + randomDouble()) / (m_width - 1);
                double y = ((double) j + randomDouble()) / (m_height - 1);
                Ray r = m_cam.sendRay(x, y);
                pixel_color += rayColor(r);
            }
            m_screen_buf[i][j] = clamp((sqrt((1.0 / m_samples_per_pixel) * pixel_color)), 0.0, 0.999);
        }
    }
    DEBUG("thread " << thread_idx << " has finished");
}

int Renderer::render(int samples, int bounces)
{   
    m_samples_per_pixel = samples;
    m_max_bounces = bounces;
    auto start_chrono = std::chrono::high_resolution_clock::now();
    
    DEBUG("Rendering on " << m_thread_amount << " cores");

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
    DEBUG("Rendering done, took: " << (double) duration.count() / 1000 << " seconds");

    delete[] threads;
    return 0;
}

int Renderer::writeToFile(std::string file)
{
    return Bmp::write(m_screen_buf, file, m_width, m_height);

#if 0
    // PPM file format:  

    DEBUG("Writing to file '" << file << "'");
    std::ofstream output;
    output.open(file);
    
    if (!output.is_open())
        return -1;
    
    //header
    output << "P3\n" << m_width << "\n" << m_height << "\n255\n";

    for (int j = m_height - 1; j >= 0; --j)
    {
        for (int i = 0; i < m_width; ++i)
        {
            Color &color = m_screen_buf[i][j];
            output << static_cast<int> (255.999 * color[0]) << ' '
                   << static_cast<int> (255.999 * color[1]) << ' '
                   << static_cast<int> (255.999 * color[2]) << '\n';
        }
    }

    DEBUG("done");
    return 0;
#endif
}