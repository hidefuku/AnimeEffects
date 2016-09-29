#include "util/TreeUtil.h"
#include "util/MathUtil.h"
#include "core/BoneKey.h"
#include "core/Project.h"
#include "core/TimeKeyBlender.h"
#include "core/ObjectNodeUtil.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
BoneKey::Data::Data()
    : mTopBones()
{
}

BoneKey::Data::Data(const Data& aRhs)
{
    for (const Bone2* bone : aRhs.topBones())
    {
        mTopBones.push_back(util::TreeUtil::createClone(bone));
    }
}

BoneKey::Data& BoneKey::Data::operator=(const Data& aRhs)
{
    deleteAll();

    for (const Bone2* bone : aRhs.topBones())
    {
        mTopBones.push_back(util::TreeUtil::createClone(bone));
    }
    return *this;
}

BoneKey::Data::~Data()
{
    deleteAll();
}

QList<Bone2*>& BoneKey::Data::topBones()
{
    return mTopBones;
}

const QList<Bone2*>& BoneKey::Data::topBones() const
{
    return mTopBones;
}

void BoneKey::Data::deleteAll()
{
    qDeleteAll(mTopBones);
    mTopBones.clear();
}

bool BoneKey::Data::isBinding(const ObjectNode& aNode) const
{
    for (auto topBone : mTopBones)
    {
        Bone2::ConstIterator itr(topBone);
        while (itr.hasNext())
        {
            if (itr.next()->isBinding(aNode)) return true;
        }
    }
    return false;
}

Bone2* BoneKey::Data::findBinderBone(const ObjectNode& aNode)
{
    for (auto topBone : mTopBones)
    {
        Bone2::Iterator itr(topBone);
        while (itr.hasNext())
        {
            auto bone = itr.next();
            if (bone->isBinding(aNode)) return bone;
        }
    }
    return nullptr;
}

//-------------------------------------------------------------------------------------------------
BoneKey::Cache::Cache()
    : mInfluence()
    , mNode()
    , mInnerMtx()
    , mFrameSign()
{
    static const int kMaxBoneCount = 32;
    mInfluence.setMaxBoneCount(kMaxBoneCount);
}

void BoneKey::Cache::setNode(ObjectNode& aNode)
{
    mNode = aNode.pointee();
}

//-------------------------------------------------------------------------------------------------
BoneKey::BindingCache::BindingCache()
    : node()
    , boneIndex()
    , innerMtx()
{
}

//-------------------------------------------------------------------------------------------------
BoneKey::BoneKey()
    : mData()
    , mCaches()
    , mCacheOwner()
    , mBindingCaches()
{
}

BoneKey::~BoneKey()
{
    destroyCaches();
}

void BoneKey::updateCaches(Project& aProject, const QList<Cache*>& aTargets)
{
    if (aTargets.isEmpty()) return;

    auto owner = mCacheOwner.get();
    XC_PTR_ASSERT(owner);

    auto time = aProject.currentTimeInfo();
    time.frame.set(this->frame());

    QWriteLocker lock(&aProject.objectTree().timeCacheLock().working);

    // update bone influence map
    for (auto cache : aTargets)
    {
        XC_PTR_ASSERT(cache->node());
        auto& node = *cache->node();

        const LayerMesh* mesh = TimeKeyBlender::getAreaMesh(node, time);
        cache->setFrameSign(mesh->frameSign());

        if (!mesh || mesh->vertexCount() <= 0) continue;

        // set world matrix
        {
            auto innerMtx = TimeKeyBlender::getRelativeMatrix(node, time, owner);
            innerMtx.translate(-ObjectNodeUtil::getCenterOffset3D(node));
            cache->setInnerMatrix(innerMtx);
        }

        BoneInfluenceMap& map = cache->influence();
        // allocate if necessary
        map.allocate(mesh->vertexCount(), false);
        // request writing
        map.writeAsync(
                    aProject, mData.topBones(),
                    cache->innerMatrix(), *mesh);
    }

#ifndef UNUSE_PARALLEL
    // wake all worker threads
    aProject.paralleler().wakeAll();
#endif

    // update binding caches
    {
        mBindingCaches.clear();
        for (auto topBone : mData.topBones())
        {
            Bone2::Iterator itr(topBone);
            while (itr.hasNext())
            {
                auto bone = itr.next();
                for (auto node : bone->bindingNodes())
                {
                    BindingCache bc;
                    bc.node = node;
                    bc.boneIndex = PosePalette::getBoneIndex(mData, *bone);
                    XC_ASSERT(bc.boneIndex >= 0);
                    auto innerMtx = TimeKeyBlender::getRelativeMatrix(*node, time, owner);
                    //innerMtx.translate(-ObjectNodeUtil::getCenterOffset3D(*node));
                    bc.innerMtx = innerMtx;
                    mBindingCaches.push_back(bc);
                }
            }
        }
    }
}

void BoneKey::updateCaches(Project& aProject, ObjectNode& aOwner, const QVector<ObjectNode*>& aUniqueRoots)
{
    QList<Cache*> caches;

    for (auto root : aUniqueRoots)
    {
        XC_PTR_ASSERT(root);
        if (!util::TreeUtil::leftContainsRight(aOwner, *root))
        {
            continue;
        }

        for (ObjectNode::Iterator itr(root); itr.hasNext();)
        {
            ObjectNode* node = itr.next();
            XC_PTR_ASSERT(node);

            Cache* cache = findCache(*node);

            if (cache && !caches.contains(cache))
            {
                caches.push_back(cache);
            }
        }
    }

    XC_ASSERT(!mCacheOwner || mCacheOwner.get() == &aOwner);
    mCacheOwner = aOwner.pointee();

    updateCaches(aProject, caches);
}

void BoneKey::resetCacheListRecursive(const TimeInfo& aTime, ObjectNode& aNode, CacheList& aNewList)
{
    bool useMe = true;

    /// @todo The node which has zero vertex mesh should contain to cache list,
    /// otherwise the timeline modifier should call resetCaches instead of updateCaches.
    {
        const LayerMesh* mesh = TimeKeyBlender::getAreaMesh(aNode, aTime);
        if (!mesh || mesh->vertexCount() <= 0) useMe = false;
    }

    // ignore
    {
        const BoneKey* areaBone = TimeKeyBlender::getAreaBone(aNode, aTime);
        // there are more local bone keys.
        if (areaBone && areaBone != this) return;
    }
    // ignore
    if (mData.isBinding(aNode)) return;

    if (useMe)
    {
        // find a cache from old list
        Cache* cache = popCache(aNode);

        // create new cache
        if (!cache)
        {
            cache = new Cache();
            // set node
            cache->setNode(aNode);
        }

        aNewList.push_back(cache);
    }

    for (auto child : aNode.children())
    {
        resetCacheListRecursive(aTime, *child, aNewList);
    }
}

void BoneKey::resetCaches(Project& aProject, ObjectNode& aOwner)
{
#if 0
    // temp list
    CacheList newCaches;

    // find valid caches
    for (ObjectNode::Iterator itr(&aOwner); itr.hasNext();)
    {
        ObjectNode* node = itr.next();
        XC_PTR_ASSERT(node);

        /// @todo The node which has zero vertex mesh should contain to cache list,
        /// otherwise the timeline modifier should call resetCaches instead of updateCaches.
        {
            auto time = aProject.currentTimeInfo();
            time.frame.set(this->frame());
            const LayerMesh* mesh = TimeKeyBlender::getAreaMesh(*node, time);
            if (!mesh || mesh->vertexCount() <= 0) continue;
        }

        // find a cache from old list
        Cache* cache = popCache(*node);

        // create new cache
        if (!cache)
        {
            cache = new Cache();
            // set node
            cache->setNode(*node);
        }

        newCaches.push_back(cache);
    }
#else
    // temp list
    CacheList newCaches;
    // make new cache list (recycle old cache)
    {
        auto time = aProject.currentTimeInfo();
        time.frame.set(this->frame());
        resetCacheListRecursive(time, aOwner, newCaches);
    }
#endif

    // destroy unuse caches
    destroyCaches();

    // update list
    for (auto cache : newCaches)
    {
        mCaches.push_back(cache);
    }

    // set owner
    mCacheOwner = aOwner.pointee();

    updateCaches(aProject, mCaches);
}

BoneKey::Cache* BoneKey::popCache(ObjectNode& aNode)
{
    for (auto itr = mCaches.begin(); itr != mCaches.end(); ++itr)
    {
        Cache* cache = *itr;
        if (cache->node() && cache->node() == &aNode)
        {
            mCaches.erase(itr);
            return cache;
        }
    }
    return nullptr;
}

BoneKey::Cache* BoneKey::findCache(const ObjectNode& aNode) const
{
    for (auto itr = mCaches.begin(); itr != mCaches.end(); ++itr)
    {
        Cache* cache = *itr;
        if (cache->node() && cache->node() == &aNode)
        {
            return cache;
        }
    }
    return nullptr;
}

void BoneKey::destroyCaches()
{
    qDeleteAll(mCaches);
    mCaches.clear();
    mCacheOwner.reset();
}

bool BoneKey::serialize(Serializer& aOut) const
{
    // top bone count
    aOut.write(mData.topBones().count());

    // serialize all bones
    for (auto topBone : mData.topBones())
    {
        XC_PTR_ASSERT(topBone);
        if (!serializeBone(aOut, topBone))
        {
            return false;
        }
    }

    // cache owner
    aOut.writeID(mCacheOwner.get());

    // cache count
    aOut.write(mCaches.count());

    // each caches
    for (auto cache : mCaches)
    {
        aOut.writeID(cache->node());
        aOut.write(cache->innerMatrix());
        aOut.write(cache->frameSign());

        if (!cache->influence().serialize(aOut))
        {
            return false;
        }
    }

    // binding cache count
    aOut.write(mBindingCaches.count());

    // each binding caches
    for (auto& cache : mBindingCaches)
    {
        aOut.writeID(cache.node);
        aOut.write(cache.boneIndex);
        aOut.write(cache.innerMtx);
    }

    return aOut.checkStream();
}

bool BoneKey::serializeBone(Serializer& aOut, const Bone2* aBone) const
{
    if (!aBone) return true;

    // child count
    aOut.write((int)aBone->children().size());

    // serialize bone
    if (!aBone->serialize(aOut))
    {
        return false;
    }

    // iterate children
    for (auto child : aBone->children())
    {
        if (!serializeBone(aOut, child))
        {
            return false;
        }
    }

    return aOut.checkStream();
}

bool BoneKey::deserialize(Deserializer& aIn)
{
    mData.deleteAll();
    destroyCaches();
    mBindingCaches.clear();

    aIn.pushLogScope("BoneKey");

    // top bone count
    int topBoneCount = 0;
    aIn.read(topBoneCount);
    if (topBoneCount < 0)
    {
        return aIn.errored("invalid top bone count");
    }

    // deserialize all bones
    for (int i = 0; i < topBoneCount; ++i)
    {
        Bone2* topBone = new Bone2();
        mData.topBones().push_back(topBone);

        if (!deserializeBone(aIn, topBone))
        {
            return false;
        }
    }

    // cache owner
    {
        auto solver = [=](void* aPtr) {
            ObjectNode* node = static_cast<ObjectNode*>(aPtr);
            if (node) this->mCacheOwner = node->pointee();
        };
        if (!aIn.orderIDData(solver))
        {
            return aIn.errored("invalid cache owner reference id");
        }
    }

    // cache count
    int cacheCount = 0;
    aIn.read(cacheCount);
    if (cacheCount < 0)
    {
        return aIn.errored("invalid cache count");
    }

    // each caches
    for (int i = 0; i < cacheCount; ++i)
    {
        Cache* cache = new Cache();
        mCaches.push_back(cache);

        // node
        {
            auto solver = [=](void* aPtr) {
                ObjectNode* node = static_cast<ObjectNode*>(aPtr);
                if (node) cache->setNode(*node);
            };
            if (!aIn.orderIDData(solver))
            {
                return aIn.errored("invalid cache reference id");
            }
        }

        // inner matrix
        QMatrix4x4 innerMtx;
        aIn.read(innerMtx);
        cache->setInnerMatrix(innerMtx);

        // frame sign
        Frame frameSign;
        aIn.read(frameSign);
        cache->setFrameSign(frameSign);

        // influence
        if (!cache->influence().deserialize(aIn))
        {
            return false;
        }
    }

    // binding cache count
    int bindingCacheCount = 0;
    aIn.read(bindingCacheCount);
    if (bindingCacheCount < 0)
    {
        return aIn.errored("invalid binding cache count");
    }

    // each binding caches
    for (int i = 0; i < bindingCacheCount; ++i)
    {
        mBindingCaches.push_back(BindingCache());
        BindingCache* cache = &(mBindingCaches.back());

        // node
        {
            auto solver = [=](void* aPtr) {
                ObjectNode* node = static_cast<ObjectNode*>(aPtr);
                if (node) cache->node = node;
            };
            if (!aIn.orderIDData(solver))
            {
                return aIn.errored("invalid binding cache reference id");
            }
        }

        // bone index
        aIn.read(cache->boneIndex);

        // inner matrix
        aIn.read(cache->innerMtx);

        if (aIn.failure())
        {
            return aIn.errored("stream error");
        }
    }

    aIn.popLogScope();
    return aIn.checkStream();
}

bool BoneKey::deserializeBone(Deserializer& aIn, Bone2* aBone)
{
    if (!aBone) return true;

    // child count
    int childCount = 0;
    aIn.read(childCount);
    if (childCount < 0)
    {
        return aIn.errored("invalid child count");
    }

    // deserialize bone
    if (!aBone->deserialize(aIn))
    {
        return false;
    }

    // iterate children
    for (int i = 0; i < childCount; ++i)
    {
        Bone2* child = new Bone2();
        aBone->children().pushBack(child);

        if (!deserializeBone(aIn, child))
        {
            return false;
        }
    }

    return aIn.checkStream();
}

} // namespace core
