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

    double m_shutter_start;
    double m_shutter_end;

public:
    Camera(Point3 lookfrom, Point3 lookat, double aspect_ratio=16/9, double vfov=90, 
           double aperature=2.0, double focus_dist=-1, double starttime=0, double shutterspeed=1/1000,
           Direction vup=Point3(0,1,0))
    {
        if (focus_dist < 0)
            focus_dist = (lookfrom - lookat).length();
        
        m_origin = lookfrom;

        double theta = degreeToRad(vfov);
        double h = tan(theta / 2);
        m_viewport_height = 2.0 * h;
        m_viewport_width = aspect_ratio * m_viewport_height;

        m_shutter_start = starttime;
        m_shutter_end = starttime + shutterspeed;
        m_aperature = aperature;
        lookAt(lookat, vup, focus_dist);
    }
    Point3 origin() const { return m_origin; }
    Point3 horizontal() const { return m_horizontal; }
    Point3 vertical() const { return m_vertical; }
    Point3 lowerLeft() const { return m_lowerLeft; }

    double shutterStart() const { return m_shutter_start; }
    double shutterEnd() const { return m_shutter_end; }

    Ray sendRay(double x, double y) 
    {
        Point3 rd = m_aperature / 2 * randomInUnitDisk();
        Point3 offset = rd.x() * m_u + rd.y() * m_v;
        return Ray(m_origin + offset, lowerLeft() + x * horizontal() + y * vertical() - m_origin - offset,
                   randomDouble(m_shutter_start, m_shutter_end));
    }

    void lookAt(Point3 lookat, Direction vup, double focus_dist)
    {
        Direction w = normalize(m_origin - lookat);
        m_u = normalize(cross(vup, w));
        m_v = cross(w, m_u);

        m_horizontal = focus_dist * m_viewport_width * m_u;
        m_vertical = focus_dist * m_viewport_height * m_v;
        m_lowerLeft = m_origin - m_horizontal / 2 - m_vertical / 2 - focus_dist * w;
    }
};