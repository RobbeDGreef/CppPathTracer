#include <core.h>
#include <renderer.h>

#include <hitables/hitable_list.h>
#include <hitables/sphere.h>
#include <hitables/triangle.h>

#include <materials/lambertian.h>
#include <materials/dielectric.h>
#include <materials/metal.h>
#include <materials/diffuse_light.h>

#include <textures/texture.h>
#include <textures/checker_texture.h>
#include <textures/noise_texture.h>
#include <textures/image_texture.h>

#include <fileformats/obj.h>
#include <fileformats/gltf.h>

#if 0
HitableList genRandom()
{
    HitableList world;

    auto mat_ground = std::make_shared<Lambertian>(std::make_shared<CheckerTexture>());
    world.add(std::make_shared<Sphere>(Point3(0,-1000, 0), 1000, mat_ground));

#if 1
    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            auto choose_mat = randomDouble();
            Point3 center(a + 0.9 * randomDouble(), 0.2, b + 0.9 * randomDouble());

            if ((center - Point3(4,0.2,0)).length() > 0.9)
            {
                std::shared_ptr<Material> sphere_material;

                if (choose_mat < 0.8)
                {
                    Color c = randomVec3() * randomVec3();
                    sphere_material = std::make_shared<Lambertian>(c);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95)
                {
                    Color c = randomVec3(0, 1);
                    double fuzz = randomDouble(0,0.5);
                    sphere_material = std::make_shared<Metal>(c, fuzz);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                } else 
                {
                    sphere_material = std::make_shared<Dielectric>(1.5);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }
#endif

    auto mat1 = std::make_shared<Dielectric>(1.5);
    world.add(std::make_shared<Sphere>(Point3(0,1,0), 1.0, mat1));
    auto mat2 = std::make_shared<Lambertian>(Color(0.4,0.2,0.1));
    world.add(std::make_shared<Sphere>(Point3(-4,1,0), 1.0, mat2));
    auto mat3 = std::make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
    world.add(std::make_shared<Sphere>(Point3(4,1,0), 1.0, mat3));

    return world;
}
#endif

Renderer createBenchmarkScene()
{
    const double aspect_ratio = 16.0 / 9.0;
    const int image_width = 800;
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    Obj object = Obj("objects/suzanne.obj");
    Scene scene = object.read();

    Camera cam = Camera(Point3(5, 2, 3), Point3(0, 0, 0), aspect_ratio, 60, 0.001, -1, 0, 1);    
    scene.setCamera(cam);

    Renderer renderer(image_width, image_height, scene, Color(0.5, 0.55, 0.9));
    renderer.render(150);
    renderer.writeToFile("test.bmp");
    return renderer;
}

Renderer renderFastBenchmarkScene()
{
    const double aspect_ratio = 16.0 / 9.0;
    const int image_width = 400;
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    GLTF gltf = GLTF("benchmarking/test.glb");
    Scene scene = gltf.read();

    Renderer renderer(image_width, image_height, scene, Color(0.5, 0.55, 0.9));
    renderer.render(150);
    renderer.writeToFile("test.bmp");
    return renderer;
}

int main(int argc, char **argv)
{
#if 0
    HitableList world = genRandom();
#endif
#if 0
    auto mat_ground = std::make_shared<Lambertian>(Color(0.8,0.8,0.0));
    auto mat_mid = std::make_shared<Dielectric>(1.5);
    auto mat_left = std::make_shared<Metal>(Color(0.8,0.6,0.2), 0.8);
    auto mat_right = std::make_shared<Metal>(Color(0.8,0.0,0.1), 0.2);
    HitableList world;
    world.add(std::make_shared<Sphere>(Point3(0,0,-1), 0.5, mat_ground));
#endif
#if 0
    HitableList world;
    auto checker = std::make_shared<CheckerTexture>(Color(0.1,0.1,0.1), Color(0.8,0.8,0.8), 10);
    world.add(std::make_shared<Sphere>(Point3(0, -1, 0), 1, std::make_shared<Lambertian>(checker)));
    world.add(std::make_shared<Sphere>(Point3(0, 1, 0), 1, std::make_shared<Lambertian>(checker)));
#endif
#if 0
    HitableList world;
    auto perlin = std::make_shared<NoiseTexture>();
    world.add(std::make_shared<Sphere>(Point3(0, 0, 0), 1, std::make_shared<Lambertian>(perlin)));
    world.add(std::make_shared<Sphere>(Point3(0, -101, 0), 100, std::make_shared<Lambertian>(perlin)));
#endif

#if 0
    HitableList world;
    auto doge = std::make_shared<ImageTexture>("textures/earthmap.bmp");
    //auto doge = std::make_shared<SolidColor>(Color(1,0,0));
    world.add(std::make_shared<Sphere>(Point3(0, 0, 0), 1, std::make_shared<Lambertian>(doge)));
    world.add(std::make_semptyhared<Sphere>(Point3(0, -101, 0), 100, std::make_shared<Lambertian>(doge)));
#endif

#if 0
    HitableList world;
    auto doge = std::make_shared<ImageTexture>("textures/earthmap.bmp");
    auto emis = std::make_shared<DiffuseLight>(Color(20,20,20));
    //auto doge = std::make_shared<SolidColor>(Color(1,0,0));
    world.add(std::make_shared<Sphere>(Point3(0, 0, 0), 1, std::make_shared<Lambertian>(doge)));
    world.add(std::make_shared<Sphere>(Point3(0, -101, 0), 100, std::make_shared<Lambertian>(doge)));
    world.add(std::make_shared<Sphere>(Point3(0,1,-1), 0.1, emis));
#endif

#if 0
    HitableList world;
    auto mat = std::make_shared<Lambertian>(Color(1,0,0));
    world.add(std::make_shared<Triangle>(Point3(0,0,0), Point3(0.5,1,1), Point3(0.2,0,1), mat));
#endif

#if 0
    HitableList world;
    auto mat = std::make_shared<Lambertian>(Color(1,0,0));
    world.add(std::make_shared<Triangle>(Point3(0,0,0), Point3(1,1,0), Point3(0.5,0,0), mat));
#endif

    renderFastBenchmarkScene();
    return 0;
}