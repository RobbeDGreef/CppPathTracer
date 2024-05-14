#pragma once

#include <vec3.h>
#include <ray.h>
#include <random.h>

class Camera
{
private:
    Point3 m_origin;
    double m_viewport_width;
    double m_viewport_height;
    double m_focal_length = 1.0;
    double m_aperature;

    Point3 m_horizontal;
    Point3 m_vertical;
    Point3 m_lowerLeft;

    Direction m_u;
    Direction m_v;

    double m_near_plane;
    double m_far_plane;

public:
    Camera(Point3 lookfrom, Point3 lookat, double aspect_ratio = 16 / 9, double vfov = 90,
           double aperature = 2.0, double focus_dist = -1,
           Direction vup = Point3(0, 1, 0), double near_plane = 0.001, double far_plane = inf)
    {
        if (focus_dist < 0)
            focus_dist = (lookfrom - lookat).length();

        m_origin = lookfrom;

        double theta = degreeToRad(vfov);
        double h = tan(theta / 2);
        m_viewport_height = 2.0 * h;
        m_viewport_width = aspect_ratio * m_viewport_height;

        m_near_plane = near_plane;
        m_far_plane = far_plane;
        m_aperature = aperature;
        lookAt(lookat, vup, focus_dist);
    }
    Point3 origin() const { return m_origin; }
    Point3 horizontal() const { return m_horizontal; }
    Point3 vertical() const { return m_vertical; }
    Point3 lowerLeft() const { return m_lowerLeft; }

    double nearplane() const { return m_near_plane; }
    double farplane() const { return m_far_plane; }

    Ray sendRay(double x, double y)
    {
        Point3 rd = m_aperature / 2 * randomGen.getPoint3InUnitDisk();
        Point3 offset = rd.x() * m_u + rd.y() * m_v;
        Direction dir = lowerLeft() + x * horizontal() + y * vertical() - m_origin - offset;
        return Ray(m_origin + offset, normalize(dir));
    }

    void lookAt(Point3 lookat, Direction vup, double focus_dist)
    {
        // BUG: if the camera is looking perfectly upwards, the normalize() in the next line will cause a
        // divide by zero and thus all rays will have direction (nan, nan, nan)

        Direction w = normalize(m_origin - lookat);
        m_u = normalize(cross(vup, w));
        m_v = cross(w, m_u);

        m_horizontal = focus_dist * m_viewport_width * m_u;
        m_vertical = focus_dist * m_viewport_height * m_v;
        m_lowerLeft = m_origin - m_horizontal / 2 - m_vertical / 2 - focus_dist * w;
    }
};