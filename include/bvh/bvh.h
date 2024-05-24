#pragma once

#include <hitables/hitable.h>
#include <hitables/hitable_list.h>
#include <config.h>
#include <unordered_map>
#include <mutex>

#define DOUBLE_HASHED_PRECISSION 10000.0
#define FIRST_HIT_CACHE_FRAC 0.05
class HashableVec3d
{
private:
    int64_t m_e[3];

public:
    int64_t x() const { return m_e[0]; }
    int64_t y() const { return m_e[1]; }
    int64_t z() const { return m_e[2]; }

    static int64_t removeDoublePrecission(double x)
    {
        return (int64_t)(x * DOUBLE_HASHED_PRECISSION);
    }

    HashableVec3d(const Vec3d &v) : HashableVec3d(v.x(), v.y(), v.z()) {}
    HashableVec3d(double x, double y, double z)
    {
        m_e[0] = removeDoublePrecission(x);
        m_e[1] = removeDoublePrecission(y);
        m_e[2] = removeDoublePrecission(z);
    }

    bool operator==(const HashableVec3d &other) const
    {
        return x() == other.x() && y() == other.y() && z() == other.z();
    }

    std::string to_string() const
    {
        return std::to_string((double)x() / DOUBLE_HASHED_PRECISSION) + " " +
               std::to_string((double)y() / DOUBLE_HASHED_PRECISSION) + " " +
               std::to_string((double)z() / DOUBLE_HASHED_PRECISSION);
    }
};

template <>
class std::hash<HashableVec3d>
{
public:
    size_t operator()(const HashableVec3d &k) const
    {
        size_t out = 0;
        for (int i = 0; i < 3; i++)
        {
            out ^= std::hash<size_t>()(k.x()) + 0x9e3779b9 + (out << 6) + (out >> 2);
        }

        return out;
    }
};

class BvhNode;

class HitCacheRecord
{
private:
    std::vector<Hitable const *> m_objects;

public:
    HitCacheRecord() {
        m_objects.reserve(5);
    }

    void add(Hitable const* object)
    {
        m_objects.push_back(object);
    }

    bool contains(Hitable const* object)
    {
        for (const auto& obj : m_objects)
        {
            if (obj == object) return true;
        }
        return false;
    }

    bool hit(const Ray &ray, double t_min, double t_max, HitRecord &rec) const
    {
        // TODO: code duplication from hitable list

        HitRecord rec_tmp;
        double closest_hit = t_max;
        bool has_hit = false;

        for (const auto &hitable : m_objects)
        {
            // Try to find a closer hit by setting tmax to closest hit found yet
            if (hitable->hit(ray, t_min, closest_hit, rec_tmp))
            {
                closest_hit = rec_tmp.t;
                rec = rec_tmp;
                has_hit = true;
            }
        }

        return has_hit;
    }
};

class BvhManager : public Hitable
{
private:
    HitablePtr m_top;

    uint8_t *m_nodes = nullptr;
    uint8_t *m_nodes_end = nullptr;

    int m_cache_cutoff_sample;
    std::vector<std::vector<HitCacheRecord>> m_cache;

public:
    BvhManager() {}
    BvhManager(const HitableList &list, int width, int height, int samples_per_pixel);

    BvhNode *allocate_node();

#if BVH_FIRST_HIT_CACHING
    bool cachedHit(int x, int y, int sample, const Ray &ray, double t_min, double t_max, HitRecord &rec);
#endif
    bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
    bool boundingBox(AABB &bounding_box) const override;
};


class BvhNode : public Hitable
{
private:
    HitablePtr m_left = nullptr;
    HitablePtr m_right = nullptr;
    AABB m_box;

public:
    BvhNode() {}
    BvhNode(const HitableList &list, BvhManager &manager)
        : BvhNode(list.objects(), 0, list.objects().size(), manager) {}

    BvhNode(const std::vector<HitablePtr> &objects, int start, int end, BvhManager &manager);

#if BVH_SAH
    static HitablePtr createTree(const std::vector<HitablePtr> &objects, BvhManager &manager);
#endif

    bool is_uninitialized() const
    {
        return m_left == nullptr || m_right == nullptr;
    }
    bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
    bool boundingBox(AABB &bounding_box) const override;
};