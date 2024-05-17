#include <core.h>
#include <vec4.h>
#include <debug.h>
#include <fileformats/gltf.h>
#include <hitables/triangle.h>
#include <materials/lambertian.h>
#include <materials/pbr.h>
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
        location = Point3(x, y, z) * GLTF_UNIT_TO_RT_UNIT;
    }
    return location;
}

Quaternion GLTF::parseNodeRotation(json &node)
{
    double x, y, z, w;
    Quaternion q = Quaternion(0, 0, 0, 1);

    if (node.contains("rotation"))
    {
        auto rot = node["rotation"];
        rot[0].get_to(x);
        rot[1].get_to(y);
        rot[2].get_to(z);
        rot[3].get_to(w);

        q = Quaternion(x, y, z, w);
    }

    return q;
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

    // Default lookAt is just in the forward direction
    Direction cameraLookDirection = Direction(0, 0, 1);
    cameraLookDirection = parseNodeRotation(node) * cameraLookDirection;

    Point3 lookAt = location - cameraLookDirection;
    scene.setCamera(Camera(location, lookAt, aspect_ratio, yfov, 0.00001));
}

void *GLTF::getBufferviewData(json file, char *bin_data, int bufferview_idx)
{
    size_t offset;
    json bufferview = file["bufferViews"][bufferview_idx];
    bufferview["byteOffset"].get_to(offset);

    return bin_data + offset;
}

std::shared_ptr<Material> GLTF::parseMaterial(json file, int mat_idx, bool &is_emissive)
{
    auto material = file["materials"][mat_idx];
    auto pbr = material["pbrMetallicRoughness"];

    std::shared_ptr<Texture> emission = std::make_shared<SolidColor>(Color(0));
    double r = 1, g = 1, b = 1, metallic = 1, roughness = 1, emissionStrength = 0, transmission = 0;

    if (material.contains("emissiveFactor"))
    {
        auto emissiveFactor = material["emissiveFactor"];
        emissiveFactor[0].get_to(r);
        emissiveFactor[1].get_to(g);
        emissiveFactor[2].get_to(b);
        emissionStrength = 1;
        is_emissive = true;
        emission = std::make_shared<SolidColor>(r, g, b);
    }

    // Handle extensions to the regular GLTF specification
    if (material.contains("extensions"))
    {
        auto extensions = material["extensions"];

        if (extensions.contains("KHR_materials_emissive_strength"))
        {
            auto em = material["extensions"]["KHR_materials_emissive_strength"];
            em["emissiveStrength"].get_to(emissionStrength);

            // TODO: maybe we should check if emissionStrength is above some threshold.
            is_emissive = true;
        }

        if (extensions.contains("KHR_materials_transmission")) {
            extensions["KHR_materials_transmission"]["transmissionFactor"].get_to(transmission);
        }
    }

    if (pbr.contains("baseColorFactor"))
    {
        pbr["baseColorFactor"][0].get_to(r);
        pbr["baseColorFactor"][1].get_to(g);
        pbr["baseColorFactor"][2].get_to(b);
    }

    if (pbr.contains("metallicFactor"))
    {
        pbr["metallicFactor"].get_to(metallic);
    }

    if (pbr.contains("roughnessFactor"))
    {
        pbr["roughnessFactor"].get_to(roughness);
    }

    DEBUG("Material with roughness " << roughness << " metallic " << metallic << " transmission " << transmission << " emission strength " << emissionStrength);
    return std::make_shared<PBR>(std::make_shared<SolidColor>(r, g, b), roughness, metallic >= 0.1, transmission, emission, emissionStrength);
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

    assert(component_type == GLTF_ACCESSOR_COMPTYPE_USHORT || component_type == GLTF_ACCESSOR_COMPTYPE_UINT);
    assert(type == "SCALAR");

    uint32_t *indices = new uint32_t[indices_count];

    if (component_type == GLTF_ACCESSOR_COMPTYPE_USHORT)
    {
        uint16_t *temp = static_cast<uint16_t *>(getBufferviewData(file, bin_data, bufferview_idx));

        for (size_t i = 0; i < indices_count; i++)
        {
            indices[i] = temp[i];
        }
    }
    else
    {
        memcpy(indices, getBufferviewData(file, bin_data, bufferview_idx), sizeof(uint32_t) * indices_count);
    }

    bool is_emissive = false;
    auto mat = parseMaterial(file, material_idx, is_emissive);

    auto list = std::make_shared<HitableList>();
    for (size_t i = 0; i < indices_count; i += 3)
    {
        auto triangle = std::make_shared<Triangle>(
            positions[indices[i + 0]].toPoint3() * GLTF_UNIT_TO_RT_UNIT,
            positions[indices[i + 1]].toPoint3() * GLTF_UNIT_TO_RT_UNIT,
            positions[indices[i + 2]].toPoint3() * GLTF_UNIT_TO_RT_UNIT,
            mat);

        list->add(triangle);
    }

    // If this material is emissive, it should be added to the lights
    scene.getHitableList().add(list);

    if (is_emissive)
    {
        scene.getLightList().push_back(list);
    }

    delete[] indices;
}

void GLTF::read(Scene& scene)
{
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

    char *json_data = new char[chunk.length + 1];
    json_data[chunk.length] = '\0';
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
}