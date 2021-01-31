#include <core.h>
#include <renderer.h>

#include <hitable/hitable_list.h>
#include <hitable/sphere.h>
#include <hitable/moving_sphere.h>

#include <materials/lambertian.h>
#include <materials/dielectric.h>
#include <materials/metal.h>

HitableList genRandom()
{
    HitableList world;

    auto mat_ground = std::make_shared<Lambertian>(Color(.5,.5,.5));
    world.add(std::make_shared<Sphere>(Point3(0,-1000, 0), 1000, mat_ground));

    # if 1
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

int main(int argc, char **argv)
{
    const double aspect_ratio = 16.0 / 9.0;
    const int image_width = 800;
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    DEBUG("Rendering image of size: " << image_width << "x" << image_height);
    
    auto mat_ground = std::make_shared<Lambertian>(Color(0.8,0.8,0.0));
    auto mat_mid = std::make_shared<Dielectric>(1.5);
    auto mat_left = std::make_shared<Metal>(Color(0.8,0.6,0.2), 0.8);
    auto mat_right = std::make_shared<Metal>(Color(0.8,0.0,0.1), 0.2);
    
    HitableList world = genRandom();
    //HitableList world;
    //world.add(std::make_shared<Sphere>(Point3(0,0,-1), 0.5, mat_ground));

    Camera cam(Point3(13,2,3), Point3(0,0,0), aspect_ratio, 20, 0.1, 10.0, 0, 1);
    Renderer renderer(image_width, image_height, world, cam);
    renderer.render(200);
    renderer.writeToFile("test.bmp");
 
    return 0;
}