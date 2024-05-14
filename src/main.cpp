#include <core.h>
#include <renderer.h>

#include <hitables/hitable_list.h>
#include <hitables/sphere.h>
#include <hitables/triangle.h>

#include <materials/lambertian.h>
#include <materials/dielectric.h>
#include <materials/metal.h>
#include <materials/diffuse_light.h>
#include <materials/uber.h>

#include <textures/texture.h>
#include <textures/checker_texture.h>
#include <textures/noise_texture.h>
#include <textures/image_texture.h>

#include <lights/point_light.h>

#include <fileformats/obj.h>
#include <fileformats/gltf.h>

Renderer renderFastMaterialTestScene()
{
    const double aspect_ratio = 16.0 / 9.0;
    const int image_width = 800;
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    Scene scene;
    auto mat = std::make_shared<Uber>(std::make_shared<SolidColor>(1, 0, 0), 1, 0, std::make_shared<SolidColor>(0), 0);
    auto sphere = std::make_shared<Sphere>(Point3(0,0,0), 1, mat);
    scene.getHitableList().add(sphere);


    auto mat2 = std::make_shared<Uber>(std::make_shared<SolidColor>(1, 1, 1), 0.1, 1, std::make_shared<SolidColor>(0), 0);
    auto sphere2 = std::make_shared<Sphere>(Point3(-2.5, 0, 0), 1, mat2);
    scene.getHitableList().add(sphere2);

    // Light
#if 1
    auto emit = std::make_shared<Uber>(std::make_shared<SolidColor>(1), 0, 0, std::make_shared<SolidColor>(1), 500);
    auto lightSphere = std::make_shared<Sphere>(Point3(4, 2, 7), 0.5, emit);
    scene.getHitableList().add(lightSphere);
#else

    auto light = std::make_shared<PointLight>(std::make_shared<SolidColor>(1, 1, 1), 10, 0.5, Point3(4, 2, 7));
    scene.getLightList().push_back(light);
#endif

    Camera cam = Camera(Point3(0, 0, 5), Point3(0, 0, 0), aspect_ratio, 60, 0.00001);    
    scene.setCamera(cam);

    Renderer renderer(image_width, image_height, scene, Color(0.05, 0.05, 0.05));
    renderer.render(500);
    renderer.writeToFile("test.bmp");
    return renderer;
}

Renderer renderFastBenchmarkScene()
{
    const double aspect_ratio = 16.0 / 9.0;
    const int image_width = 400;
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    GLTF gltf = GLTF("benchmarking/3_balls.glb");
    Scene scene = gltf.read();

    Renderer renderer(image_width, image_height, scene, Color(0,0,0));
    renderer.render(150);
    renderer.writeToFile("test.bmp");
    return renderer;
}

Renderer renderFastCornellBenchmarkScene()
{
    const double aspect_ratio = 1;
    const int image_width = 400;
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    GLTF gltf = GLTF("benchmarking/cornell/cornell_boxes_2.glb");
    Scene scene = gltf.read();

    Renderer renderer(image_width, image_height, scene, Color(0.051, 0.051, 0.051));
    // TODO: BUG: high sample counts introduce a lot of high pitched noise?
    renderer.render(1500, 5);
    renderer.writeToFile("test.bmp");
    return renderer;
}


int main(int argc, char **argv)
{
    //renderFastBenchmarkScene();
    //renderFastMaterialTestScene();
    renderFastCornellBenchmarkScene();
    return 0;
}