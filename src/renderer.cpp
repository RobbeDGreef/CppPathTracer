#include <renderer.h>
#include <bmp.h>
#include <hitables/hitable.h>
#include <random.h>
#include <materials/material.h>
#include <chrono>
#include <core.h>
#include <scene.h>
#include <pdfs/lightpdf.h>
#include <pdfs/mixturepdf.h>

#define RAY_NEAR_CLIP 0.001
#define RAY_FAR_CLIP inf
#define MAX_SAMPLE_OUTPUT_COLOR 20

void Renderer::set_dimensions(int width, int height)
{
    m_width = width;
    m_height = height;

    m_screen_buf = std::make_unique<ColorArray>(width, height);
}

void Renderer::generate_bvh()
{
    m_world = std::move(BvhNode(m_scene.getHitableList()));
}

void Renderer::set_background_color(Color bg)
{
    m_background = bg;
}

void Renderer::set_threads(int threads)
{
    m_thread_amount = threads;
}

void Renderer::set_samples_per_pixel(int samples)
{
    m_samples_per_pixel = samples;
}

void Renderer::set_max_bounces(int max_bounces)
{
    m_max_bounces = max_bounces;
}

Color Renderer::rayColor(const Ray &r, int bounces = 0)
{
    if (bounces == m_max_bounces)
        return Color(0);

    // If we do not hit anything with this ray, we return the background color / texture
    HitRecord rec;
    if (!m_world.hit(r, RAY_NEAR_CLIP, RAY_FAR_CLIP, rec))
    {
        // TODO: implement HDRI
        return m_background;
    }

    // We did hit an object, now we calculate its color based on its material, the lights in the scene etc
    Ray scattered;

    // The emitted function returns the amount of emission the material has,
    // if this is none, we assume the output variable is not changed and thus stays 0
    Color output = Color(0);
    rec.mat->emitted(rec.u, rec.v, rec.p, output);

    // Scatter the ray to calculate the next ray
    ScatterRecord srec;
    if (!rec.mat->scatter(r, rec, srec))
    {
        return output;
    }

    double pdf_sample;
    if (srec.skip_pdf)
    {
        scattered = srec.scattered_ray;
        pdf_sample = 1;
    }
    else
    {
        // Evaluate the PDF to find the scatter direction

        auto light_pdf = std::make_shared<LightPDF>(rec.p, m_scene.getLightList().front());
        auto pdf = std::make_shared<MixturePDF>(0.5, srec.pdf, light_pdf);
        Direction dir = normalize(pdf->generate());
        scattered = Ray(rec.p, dir);
        pdf_sample = pdf->value(dir);

        srec.scattered_ray = scattered;

        // If an extreme PDF value occurs it causes extreme values in the pixel
        // due to divisions by a very small number, in theory a good monte carlo
        // simulation would still filter these out, however, the precision of
        // computers is holding us back here and we have to create some form of
        // cutoff to make sure we dont cause these extreme pixels.

        // This tends to happen when a bad direction is chosen by the LightPDF
        // which causes some epsilon somewhere to claim the ray does not hit the
        // sampled light, which then causes a 0 probability.

        if (pdf_sample < 0.001)
        {
            pdf_sample = 1;
        }
    }

    Color sample_eval = rec.mat->eval(r, rec, srec);
    output += (sample_eval * rayColor(scattered, bounces + 1)) / pdf_sample;

    // Clamp the output value to reduce fireflies. This technique is not great because
    // it introduces bias, but it does work
    output = clamp(output, 0, MAX_SAMPLE_OUTPUT_COLOR);

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

    randomGen = RandomGenerator();

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
                double x = ((double)i + randomGen.getDouble()) / (m_width - 1);
                double y = ((double)j + randomGen.getDouble()) / (m_height - 1);
                Ray r = m_scene.getCamera().sendRay(x, y);
                pixel_color += rayColor(r);
            }
            Color linear_color = pixel_color / m_samples_per_pixel;
            Color gamma_corrected = pow(linear_color, 1.0 / 2.2);
            m_screen_buf->at(i)[j] = clamp(gamma_corrected, 0.0, 1.0);
        }
    }
    OUT("thread " << thread_idx << " has finished");
}

int Renderer::render()
{
    generate_bvh();

    auto start_chrono = std::chrono::high_resolution_clock::now();

    OUT("Rendering on " << m_thread_amount << " threads");
    OUT("Image size: " << m_width << "x" << m_height);
    OUT("Samples per pixel: " << m_samples_per_pixel);
    OUT("Maximum ray bounces " << m_max_bounces);

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
    return Bmp::write(m_screen_buf.get(), file, m_width, m_height);
}