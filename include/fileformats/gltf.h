#include <core.h>
#include <fileformats/input_file_format.h>
#include <stdint.h>

#include <json.h>
using json = nlohmann::json_abi_v3_11_3::json;

#define GLTF_MAGIC_BYTES 0x46546C67

#define GLTF_CHUNK_TYPE_JSON 0x4E4F534A
#define GLTF_CHUNK_TYPE_BIN  0x004E4942

#define GLTF_ACCESSOR_COMPTYPE_USHORT   5123
#define GLTF_ACCESSOR_COMPTYPE_UINT     5125
#define GLTF_ACCESSOR_COMPTYPE_FLOAT    5126

/* todo: removeme, this is just to make my ide shut up */
#ifndef PACKED
#define PACKED(X) X
#endif

PACKED(struct GLTFHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t length;
});

PACKED(struct GLTFChunk {
    uint32_t length;
    uint32_t type;
});

PACKED(
template <typename T>
struct GLTFVec3 {
    T x;
    T y;
    T z;

    Point3 toPoint3() {
        return Point3(this->x, this->y, this->z);
    }
});

class GLTF : public InputFileFormat
{
private:
    Point3 parseNodeTranslation(json& node);
    void parseCameraNode(Scene& scene, json& node, json& file);
    void parseMeshNode(Scene& scene, json& node, json& file, char* bin_data);
    void* getBufferviewData(json file, char* bin_data, int bufferview_idx);
    std::shared_ptr<Material> parseMaterial(json file, int mat_idx);

public:
    GLTF(std::string filename) : InputFileFormat(filename) {}
    Scene read();
};