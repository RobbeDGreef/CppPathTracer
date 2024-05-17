#pragma once

#include <core.h>
#include <vec3.h>
#include <hitables/hitable_list.h>
#include <fileformats/input_file_format.h>

class Obj : public InputFileFormat
{
private:
    std::vector<Point3> m_verts;

private:
    Point3 parseVec(const std::vector<std::string> &parts, int start_index) const;
    Point3 getVert(const std::string vert) const;

public:
    Obj(std::string filename) : InputFileFormat(filename) {}
    void read(Scene& scene);
};
