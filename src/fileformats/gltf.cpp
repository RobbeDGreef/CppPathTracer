#include <core.h>
#include <debug.h>
#include <fileformats/gltf.h>
#include <hitables/triangle.h>
#include <materials/lambertian.h>
#include <textures/uv_texture.h>
#include <json.h>

using json = nlohmann::json_abi_v3_11_3::json;

Point3 GLTF::parseNodeTranslation(json &node)
{
    double x, y, z;
    Point3 location = Point3(0, 0, 0);

    // Node contains translation, extract XYZ location
    if (node.contains("translation"))
    {
        node["translation"][0].get_to(x);
        node["translation"][1].get_to(y);
        node["translation"][2].get_to(z);
        location = Point3(x, y, z);
    }
    return location;
}

void GLTF::parseCameraNode(Scene &scene, json &node, json &file)
{
    Point3 location = parseNodeTranslation(node);

    // Get the camera node
    int camera_idx;
    node["camera"].get_to(camera_idx);
    json camera = file["cameras"][camera_idx];

    // We only support perspective cameras (and no orthographic cameras)
    if (camera["type"] != "perspective")
    {
        ERROR("Only perspective cameras are supported");
        exit(1);
    }

    double aspect_ratio = 1.777;
    double yfov;

    camera["perspective"]["yfov"].get_to(yfov);
    if (camera["perspective"].contains("aspectRatio"))
    {
        camera["perspective"]["aspectRatio"].get_to(aspect_ratio);
    }

    // Transform yfov from radians to degrees
    yfov = (yfov / (2 * pi)) * 360;

    // TODO: issue with the focus distance
    // Calculate the forward vector from the quaternion rotation
    // Point3 lookAt = Point3(2 * (y*w + x*z),
    //                       2 * (z*w + x*y),
    //                       1 - 2 * (y*y + z*z));
    //

    scene.setCamera(Camera(location, Point3(0, 0, 0), aspect_ratio, yfov));
}

void *GLTF::getBufferviewData(json file, char *bin_data, int bufferview_idx)
{
    size_t offset;
    json bufferview = file["bufferViews"][bufferview_idx];
    bufferview["byteOffset"].get_to(offset);

    return bin_data + offset;
}

std::shared_ptr<Material> GLTF::parseMaterial(json file, int mat_idx) {
    return std::make_shared<Lambertian>(std::make_shared<UVTexture>());
}

void GLTF::parseMeshNode(Scene &scene, json &node, json &file, char *bin_data)
{
    // Get the mesh node
    int mesh_idx;
    node["mesh"].get_to(mesh_idx);
    json mesh = file["meshes"][mesh_idx];

    // The primitives object contains the positions of all
    // the vertices as well as texture coordinates etc.
    json primitives = mesh["primitives"][0];
    int positions_accessor_idx;
    int indices_accessor_idx;
    int material_idx;

    // All values in the primitives json object are indices into the accessor array
    primitives["attributes"]["POSITION"].get_to(positions_accessor_idx);
    primitives["indices"].get_to(indices_accessor_idx);
    primitives["material"].get_to(material_idx);

    // All data from accessors is stored in the binary part of this file.
    // It is described by bufferviews which have their types etc.
    json pos_accessor = file["accessors"][positions_accessor_idx];
    json ind_accessor = file["accessors"][indices_accessor_idx];

    // Handle the position accessor, this describes where the vertices of the mesh
    // are in 3d space (I think).
    size_t positions_count;
    int bufferview_idx;
    int component_type;
    std::string type;
    pos_accessor["componentType"].get_to(component_type);
    pos_accessor["bufferView"].get_to(bufferview_idx);
    pos_accessor["count"].get_to(positions_count);
    pos_accessor["type"].get_to(type);

    assert(component_type == GLTF_ACCESSOR_COMPTYPE_FLOAT);
    assert(type == "VEC3");

    GLTFVec3<float> *positions = static_cast<GLTFVec3<float> *>(getBufferviewData(file, bin_data, bufferview_idx));

    // Now use the indices to create triangles and add them to the hitable list

    size_t indices_count;
    ind_accessor["bufferView"].get_to(bufferview_idx);
    ind_accessor["count"].get_to(indices_count);
    ind_accessor["type"].get_to(type);
    ind_accessor["componentType"].get_to(component_type);

    assert(component_type == GLTF_ACCESSOR_COMPTYPE_USHORT);
    assert(type == "SCALAR");

    uint16_t *indices = static_cast<uint16_t *>(getBufferviewData(file, bin_data, bufferview_idx));

    auto mat = parseMaterial(file, material_idx);

    for (size_t i = 0; i < indices_count; i += 3)
    {
        DEBUG("index" << i);
        auto triangle = std::make_shared<Triangle>(
            positions[indices[i + 0]].toPoint3(),
            positions[indices[i + 1]].toPoint3(),
            positions[indices[i + 2]].toPoint3(),
            mat);
        scene.getHitableList().add(triangle);
    }
}

Scene GLTF::read()
{
    Scene scene;

    GLTFHeader header;
    GLTFChunk chunk;

    m_infile.read((char *)&header, sizeof(GLTFHeader));

    if (header.magic != GLTF_MAGIC_BYTES)
    {
        ERROR("Invalid GLTF file");
        exit(1);
    }

    if (header.version != 2)
    {
        ERROR("Only GLTF2 is supported");
        exit(1);
    }

    // Read the first two chunks, json and binary data
    m_infile.read((char *)&chunk, sizeof(GLTFChunk));

    if (chunk.type != GLTF_CHUNK_TYPE_JSON)
    {
        ERROR("Invalid GLTF file, chunks are malformed");
        exit(1);
    }

    char *json_data = new char[chunk.length];
    m_infile.read(json_data, chunk.length);

    m_infile.read((char *)&chunk, sizeof(GLTFChunk));
    if (chunk.type != GLTF_CHUNK_TYPE_BIN)
    {
        ERROR("Invalid GLTF file, chunks are malformed");
        exit(1);
    }

    char *bin_data = new char[chunk.length];
    m_infile.read(bin_data, chunk.length);

    // Interpret the json file and populate the hitable list with the scene
    json file = nlohmann::json::parse(json_data);

    // We only support a single scene in this parser

    std::int64_t scene_idx = file["scene"].template get<std::int64_t>();

    json gltf_scene = file["scenes"][scene_idx];
    for (json node_idx_json : gltf_scene["nodes"])
    {
        int node_idx;
        node_idx_json.get_to(node_idx);
        json node = file["nodes"][node_idx];

        // TODO: handle rotation as well

        if (node.contains("mesh"))
        {
            parseMeshNode(scene, node, file, bin_data);
        }
        else if (node.contains("camera"))
        {
            parseCameraNode(scene, node, file);
        }
    }

    return scene;
}