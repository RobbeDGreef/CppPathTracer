#include <bvh/bvh.h>
#include <random.h>

#include <sys/mman.h>
#include <unistd.h>

bool BvhNode::hit(const Ray &ray, double t_min, double t_max, HitRecord &rec) const
{
    if (!m_box.hit(ray, t_min, t_max))
        return false;

    bool hit_left = m_left->hit(ray, t_min, t_max, rec);
    bool hit_right = m_right->hit(ray, t_min, hit_left ? rec.t : t_max, rec);

    return hit_left || hit_right;
}

static bool hitableCompare(HitablePtr &a, HitablePtr &b, int axis)
{
    AABB a_box;
    AABB b_box;

    if (!a->boundingBox(a_box) || !b->boundingBox(b_box))
        ERROR("No bounding box could be constructed in the BVH");

    return a_box.minPoint()[axis] < a_box.minPoint()[axis];
}

#if BVH_SAH

double calculateSplitCost(const std::vector<HitablePtr> &objects, int axis, double pos)
{
    AABB left, right;
    int left_count = 0, right_count = 0;

    for (auto it = objects.begin(); it != objects.end(); ++it)
    {
        const auto& hitable = *it;
        if (hitable->center()[axis] < pos)
        {
            left_count++;
            AABB tmp;
            hitable->boundingBox(tmp);
            left = left_count == 1 ? tmp : AABB::surroundingBox(left, tmp);
        }
        else
        {
            right_count++;
            AABB tmp;
            hitable->boundingBox(tmp);
            right = right_count == 1 ? tmp : AABB::surroundingBox(right, tmp);
        }
    }

    double cost = left_count * left.surfaceArea() + right_count * right.surfaceArea();
    return cost;
}

#endif

#if BVH_SAH

HitablePtr BvhNode::createTree(const std::vector<HitablePtr> &objects, BvhManager &manager)
{
    if (objects.size() == 1)
        return objects.front();

    // Parent box is AABB of all nodes in this tree
    AABB parent_box = AABB(objects);

    // Find the best split, it is possible that not splitting is 
    // better, this will mean that the cost of the parent box is 
    // lower than any cost of a split.
    double best_cost = parent_box.surfaceArea() * objects.size();
    double best_pos = 0;
    int best_axis = -1;

    // For every axis check all objects to find the best possible 
    // split
    for (int axis = 0; axis < 3; axis++)
    {
        double begin = parent_box.minPoint()[axis];
        double end = parent_box.maxPoint()[axis];
        double interval = (end-begin) / 10;
        for (double pos = begin; pos < end; pos += interval)
        {
            double cost = calculateSplitCost(objects, axis, pos);
            if (cost < best_cost)
            {
                best_pos = pos;
                best_cost = cost;
                best_axis = axis;
            }
        }
    }

    // The best thing we can do is not split our BVH
    // and instead return a regular hitable list.
    if (best_axis == -1)
    {
        return new HitableList(objects);
    }

    // Otherwise split the BVH into two nodes
    
    // Preallocate the left and right objects list with enough spaces
    // for all the objects. This is a wasteful usage of memory, but it
    // is faster this way.
    std::vector<HitablePtr> left_objects;
    std::vector<HitablePtr> right_objects;

    for (const auto& obj : objects) {
        if (obj->center()[best_axis] < best_pos)
            left_objects.push_back(obj);
        else 
            right_objects.push_back(obj);
    }

    BvhNode* out = new(manager.allocate_node()) BvhNode();
    
    out->m_box = parent_box;
    out->m_left = createTree(left_objects, manager);
    out->m_right = createTree(right_objects, manager);

    return out;
}

#endif

BvhNode::BvhNode(const std::vector<HitablePtr> &_objects, int start, int end, BvhManager &manager)
{
    auto objects = _objects;

    int axis = randomGen.getInt(0, 2);

    auto comparator = [axis](HitablePtr &a, HitablePtr &b)
    { return hitableCompare(a, b, axis); };

    int length = end - start;

    switch (length)
    {
    case 1:
        m_left = m_right = objects[start];
        break;
    case 2:
        m_left = objects[start + 1];
        m_right = objects[start];
        if (comparator(objects[start], objects[start + 1]))
            std::swap(m_left, m_right);

        break;
    default:

        std::sort(objects.begin() + start, objects.begin() + end, comparator);

        auto mid = start + length / 2;
        m_left = manager.allocate_node();
        m_right = manager.allocate_node();
        m_left = new (m_left) BvhNode(objects, start, mid, manager);
        m_right = new (m_right) BvhNode(objects, mid, end, manager);
    }

    AABB box_left;
    AABB box_right;

    if (!m_left->boundingBox(box_left) || !m_right->boundingBox(box_right))
        ERROR("No bounding box could be constructed in the BVH");

    m_box = AABB::surroundingBox(box_left, box_right);
}



bool BvhNode::boundingBox(AABB &bounding_box) const
{
    bounding_box = m_box;
    return true;
}

bool BvhManager::hit(const Ray &ray, double t_min, double t_max, HitRecord &rec) const
{
    return m_top->hit(ray, t_min, t_max, rec);
}

#if BVH_FIRST_HIT_CACHING
bool BvhManager::cachedHit(int x, int y, int sample, const Ray &ray, double t_min, double t_max, HitRecord &rec)
{
    HitCacheRecord &cached = m_cache[x][y];

    if (sample >= m_cache_cutoff_sample)
    {
        return cached.hit(ray, t_min, t_max, rec);
    }
    else
    {
        bool hit = m_top->hit(ray, t_min, t_max, rec);

        if (hit && !cached.contains(rec.hitable))
        {
            cached.add(rec.hitable);
        }

        return hit;
    }
}
#endif

bool BvhManager::boundingBox(AABB &bounding_box) const
{
    return m_top->boundingBox(bounding_box);
}

BvhNode *BvhManager::allocate_node()
{
    // WARNING: this has absolutely NO portability and is the equivalent of
    // a watermark allocator per mapped page.
    // This also wastes some bytes if the BvhNode does not fit into the
    // page anymore but this should be relatively minimal.

    m_nodes += sizeof(BvhNode);
    if (m_nodes > m_nodes_end)
    {
        int page_size = getpagesize();

        m_nodes = static_cast<uint8_t *>(mmap(m_nodes_end, page_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
        m_nodes_end = m_nodes + page_size;

        if (reinterpret_cast<long>(m_nodes) == -1)
        {
            ERROR("Could not map page");
            exit(1);
        }

        m_nodes += sizeof(BvhNode);
    }

    return reinterpret_cast<BvhNode *>(m_nodes - sizeof(BvhNode));
}

BvhManager::BvhManager(const HitableList &list, int width, int height, int samples_per_pixel)
{
    m_cache = std::vector<std::vector<HitCacheRecord>>(width, std::vector<HitCacheRecord>(height, HitCacheRecord()));

    m_cache_cutoff_sample = static_cast<int>(static_cast<double>(samples_per_pixel) * FIRST_HIT_CACHE_FRAC);
    
#if BVH_SAH
    m_top = BvhNode::createTree(list.objects(), *this);
#else
    m_top = allocate_node();
    m_top = new (m_top) BvhNode(list.objects(), 0, list.objects().size(), *this);
#endif
}