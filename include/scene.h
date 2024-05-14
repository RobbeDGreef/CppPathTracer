#pragma once

#include <core.h>
#include <hitables/hitable_list.h>
#include <camera.h>
#include <list>

class Scene
{
private:
    std::vector<std::shared_ptr<HitableList>> m_lights;
    HitableList m_hitlist;
    Camera m_camera = Camera(Point3(0, 0, 1), Point3(0, 0, 0));

public:
    Scene() {}
    Scene(HitableList lst) : m_hitlist(lst) {}
    Scene(HitableList lst, Camera cam) : m_hitlist(lst), m_camera(cam) {}

    void setCamera(Camera cam)
    {
        m_camera = cam;
    }

    Camera &getCamera()
    {
        return m_camera;
    }

    HitableList &getHitableList()
    {
        return m_hitlist;
    }

    std::vector<std::shared_ptr<HitableList>> &getLightList()
    {
    return m_lights;
    }
};