#include <core.h>
#include <renderer.h>

#include <hitables/hitable_list.h>
#include <hitables/sphere.h>
#include <hitables/triangle.h>

#include <materials/lambertian.h>
#include <materials/dielectric.h>
#include <materials/metal.h>
#include <materials/diffuse_light.h>
#include <materials/pbr.h>

#include <textures/texture.h>
#include <textures/checker_texture.h>
#include <textures/noise_texture.h>
#include <textures/image_texture.h>

#include <fileformats/obj.h>
#include <fileformats/gltf.h>

#include <argparse/argparse.hpp>
#include <thread>

void setupGLTFBenchmarkScene(Renderer &renderer, std::string path, int width=800, double aspect_ratio=1, int samples=200)
{
    renderer.set_background_color(Color(0.051, 0.051, 0.051));
    renderer.set_dimensions(width, width*aspect_ratio);
    renderer.set_max_bounces(12);
    renderer.set_samples_per_pixel(samples);

    GLTF gltf = GLTF(path);
    gltf.read(renderer.get_scene());
}

void loadPreset(Renderer &renderer, std::string preset_name)
{
    if (preset_name == "fast_cornell_benchmark") 
        return setupGLTFBenchmarkScene(renderer, "benchmarking/cornell/cornell_boxes.glb");
    if (preset_name == "suzanne")
        return setupGLTFBenchmarkScene(renderer, "benchmarking/suzanne_on_table_hr.glb");
    if (preset_name == "suzanne_big")
        return setupGLTFBenchmarkScene(renderer, "benchmarking/suzanne_on_table_hr.glb", 1920, 9.0/16.0, 500);
    if (preset_name == "suzanne_fast")
        return setupGLTFBenchmarkScene(renderer, "benchmarking/suzanne_on_table_hr.glb", 500);
    if (preset_name == "stanford_dragon")
        return setupGLTFBenchmarkScene(renderer, "benchmarking/stanford-dragon.glb", 1200, 9.0/16.0, 500);
    if (preset_name == "stanford_dragon_glass")
        return setupGLTFBenchmarkScene(renderer, "benchmarking/stanford-dragon-glass.glb", 800, 9.0/16.0, 500);

    ERROR("Unknown preset " << preset_name);
    exit(1);
}

int main(int argc, char **argv)
{
    argparse::ArgumentParser program("Raytracer");
    program.add_argument("-t", "--threads")
        .default_value(static_cast<int>(std::thread::hardware_concurrency()))
        .help("specify the amount of threads should be used")
        .scan<'i', int>();

    program.add_argument("-s", "--samples")
        .default_value(200)
        .help("specify the amount of samples per pixel should be performed")
        .scan<'i', int>();

    program.add_argument("--width")
        .default_value(400)
        .help("specify the width of the image")
        .scan<'i', int>();

    program.add_argument("--height")
        .default_value(400)
        .help("specify the height of the image")
        .scan<'i', int>();

    program.add_argument("-b", "--bounces")
        .default_value(12)
        .help("specify the maximal amount of ray bounces")
        .scan<'i', int>();

    program.add_argument("-o", "--outfile")
        .default_value(std::string("out.bmp"))
        .help("specify the file the output image needs to be written to (BMP format)");

    program.add_argument("--preset")
        .help("specify a preset to run");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::exception &err)
    {
        ERROR("Did not understand argument(s) " << err.what());
        std::cerr << program;
        return 1;
    }

    Renderer renderer = Renderer();
    
    int samples = program.get<int>("--samples");
    int bounces = program.get<int>("--bounces");
    int threads = program.get<int>("--threads");
    int width = program.get<int>("--width");
    int height = program.get<int>("--height");
    auto preset = program.get<std::string>("--preset");

    renderer.set_threads(threads);

    if (program.present("--preset"))
    {
        loadPreset(renderer, program.get<std::string>("--preset"));
    }
    else
    {
        renderer.set_dimensions(width, height);
        renderer.set_samples_per_pixel(samples);
        renderer.set_max_bounces(bounces);
    }

    renderer.render();

    std::string outfile = program.get("--outfile");
    renderer.writeToFile(outfile);
    return 0;
}
