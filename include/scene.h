#pragma once

#include <core.h>
#include <hitables/hitable_list.h>
#include <camera.h>
#include <lights/light.h>

class Scene
{
private:
    std::list<Light> m_lights;
    HitableList m_hitlist;
    Camera m_camera = Camera(Point3(0,0,1), Point3(0,0,0));

public:
    Scene() {}
    Scene(HitableList lst) : m_hitlist(lst) {}
    Scene(HitableList lst, Camera cam) : m_hitlist(lst), m_camera(cam) {}

    void setCamera(Camera cam)
    {
        m_camera = cam;
    }

    Camera& getCamera()
    {
        return m_camera;
    }

    HitableList& getHitableList()
    {
        return m_hitlist;
    }

    std::list<Light>& getLightList()
    {
        return m_lights;
    }
};