#include <fileformats/input_file_format.h>

InputFileFormat::InputFileFormat(std::string filename) : m_infile_name(filename)
{
    m_infile.open(filename);
    if (!m_infile.is_open())
        WARN("Could not read file: " << filename);
}