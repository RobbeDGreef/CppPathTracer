#pragma once

#include <core.h>
#include <hitables/hitable_list.h>
#include <vec3.h>

class Obj
{
private:
    std::ifstream m_infile;
    std::vector<Point3> m_verts;

private:
    Point3 parseVec(const std::vector<std::string> &parts, int start_index) const;
    Point3 getVert(const std::string vert) const;

public:
    Obj(std::string filename);
    HitableList read();
};