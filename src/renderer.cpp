#include <renderer.h>
#include <random.h>
#include <bmp.h>
#include <core.h>
#include <scene.h>
#include <config.h>

#include <hitables/hitable.h>
#include <materials/material.h>
#include <pdfs/lightpdf.h>
#include <pdfs/mixturepdf.h>

#include <omp.h>

#include <chrono>

#define RAY_NEAR_CLIP 0.001
#define RAY_FAR_CLIP inf

void Renderer::set_dimensions(int width, int height)
{
    m_width = width;
    m_height = height;

    m_screen_buf = std::make_unique<ColorArray>(width, height);
}

void Renderer::generate_bvh()
{
    auto start_chrono = std::chrono::high_resolution_clock::now();
    m_world = BvhManager(m_scene.getHitableList(), m_width, m_height, m_samples_per_pixel);
    auto stop_chrono = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_chrono - start_chrono);
    OUT("BVH generation done, took: " << (double)duration.count() / 1000 << " seconds");
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

void Renderer::renderPixel(ColorArray *array, int x, int y)
{
    Color pixel_color;
    for (int s = 0; s < m_samples_per_pixel; ++s)
    {
        double x_coord = ((double)x + randomGen.getDouble()) / (m_width - 1);
        double y_coord = ((double)y + randomGen.getDouble()) / (m_height - 1);
        const Ray r = m_scene.getCamera().sendRay(x_coord, y_coord);
        pixel_color += rayColor(r);
    }
    Color linear_color = pixel_color / m_samples_per_pixel;
    Color gamma_corrected = pow(linear_color, 1.0 / 2.2);
    array->at(x)[y] = clamp(gamma_corrected, 0.0, 1.0);
}

#if THREADING_IMPLEMENTATION == THREAD_IMPL_NAIVE

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

void Renderer::renderThread(ColorArray* buffer, int thread_idx, double *percentages)
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
            renderPixel(buffer, i, j);
        }
    }
    OUT("thread " << thread_idx << " has finished");
}

#endif

#if THREADING_IMPLEMENTATION == THREAD_IMPL_OPENMP_BLOCKS

void Renderer::renderBlock(ColorArray *buffer, RenderWorkBlock work)
{
    for (int y = work.y; y < work.y_end; ++y)
    {
        for (int x = work.x; x < work.x_end; ++x)
        {
            renderPixel(buffer, x, y);
        }
    }
}

#endif

int Renderer::render()
{
    // First generate the acceleration structure
    generate_bvh();

    OUT("Rendering on " << m_thread_amount << " threads");
    OUT("Image size: " << m_width << "x" << m_height);
    OUT("Samples per pixel: " << m_samples_per_pixel);
    OUT("Maximum ray bounces " << m_max_bounces);

    auto start_chrono = std::chrono::high_resolution_clock::now();

#if USE_COLOR_BUFFER_PER_THREAD
    std::vector<std::unique_ptr<ColorArray>> buffers;
    for (int i = 0; i < m_thread_amount; i++)
    {
        buffers.push_back(std::make_unique<ColorArray>(m_width, m_height));
    }
#endif

#if THREADING_IMPLEMENTATION == THREAD_IMPL_NAIVE
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

#if USE_COLOR_BUFFER_PER_THREAD
            ColorArray *buffer = buffers[i].get();
#else
            ColorArray *buffer = m_screen_buf.get();
#endif

        threads[i] = std::thread(&Renderer::renderThread, this, buffer, i, percentages);
    }

#if ENABLE_PROGRESS_INDICATOR
    // Display the progress
    std::thread(&Renderer::calcProgress, this, percentages).detach();
#endif

    for (int i = 0; i < m_thread_amount; i++)
    {
        threads[i].join();
    }

    delete[] percentages;
    delete[] threads;

#endif

#if THREADING_IMPLEMENTATION == THREAD_IMPL_OPENMP_BLOCKS

    // Divide the work up into squares of computation and create a
    // queue where threads can take work out of

    std::queue<RenderWorkBlock> work;
    for (int x = 0; x < m_width; x += WORK_SQUARE_SIZE)
    {
        for (int y = 0; y < m_height; y += WORK_SQUARE_SIZE)
        {
            int width = x + WORK_SQUARE_SIZE >= m_width ? (m_width - x - 1) : WORK_SQUARE_SIZE;
            int height = y + WORK_SQUARE_SIZE >= m_height ? (m_height - y - 1) : WORK_SQUARE_SIZE;

            work.push(RenderWorkBlock{x, y, x + width, y + height});
        }
    }

    int total_work = work.size();
    RenderWorkQueue work_queue(work);

#if ENABLE_PROGRESS_INDICATOR
    std::thread progress([&]()
                         {
        int size = work_queue.size();
        while ((size = work_queue.size()) != 0)
        {
            OUT((1 - (double)size / total_work)*100 << "% completed");
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        } });
    progress.detach();
#endif

    // Launch some parallel instances of the raytracing algorithm
    omp_set_num_threads(m_thread_amount);
#pragma omp parallel
    {
        // Initialize the random generator for each thread
        // This randomgen is stored in thread local storage.-
        randomGen = RandomGenerator();

        std::optional<RenderWorkBlock> work = work_queue.pop();
        while (work.has_value())
        {

#if USE_COLOR_BUFFER_PER_THREAD
            int thread = omp_get_thread_num();
            ColorArray *buffer = buffers[thread].get();
#else
            ColorArray *buffer = m_screen_buf.get();
#endif

            renderBlock(buffer, work.value());

            work = work_queue.pop();
        }
    }

#endif

#if THREADING_IMPLEMENTATION == THREAD_IMPL_OPENMP_PER_PIXEL

    omp_set_num_threads(m_thread_amount);
    #pragma omp parallel for collapse(2)
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {

#if USE_COLOR_BUFFER_PER_THREAD
            int thread = omp_get_thread_num();
            ColorArray *buffer = buffers[thread].get();
#else
            ColorArray *buffer = m_screen_buf.get();
#endif

            renderPixel(buffer, x, y);
        }
    }

#endif

#if USE_COLOR_BUFFER_PER_THREAD
    // Merge the final buffers
    for (int x = 0; x < m_width; x++)
    {
        for (int y = 0; y < m_height; y++)
        {
            for (auto &&buf : buffers)
            {
                m_screen_buf->at(x)[y] += buf->at(x)[y];
            }
        }
    }
#endif

    auto stop_chrono = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_chrono - start_chrono);
    OUT("Rendering done, took: " << (double)duration.count() / 1000 << " seconds");

    return 0;
}

int Renderer::writeToFile(std::string file)
{
    return Bmp::write(m_screen_buf.get(), file, m_width, m_height);
}