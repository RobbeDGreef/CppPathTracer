#pragma once

#include <core.h>
#include <scene.h>

class InputFileFormat
{
protected:
    std::ifstream m_infile;
    std::string m_infile_name;
public:
    InputFileFormat(std::ifstream stream);
    InputFileFormat(std::string filename);
    void read(Scene& scene);
};