#include "util/TreeIterator.h"
#include "util/TreeSeekIterator.h"
#include "util/MathUtil.h"
#include "core/TimeKeyExpans.h"
#include "core/TimeKeyBlender.h"
#include "core/LayerMesh.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
class ObjectTreeSeeker : public util::ITreeSeeker<TimeKeyBlender::SeekData>
{
    bool mUseCache;
public:
    ObjectTreeSeeker(bool aUseCache)
        : mUseCache(aUseCache)
    {
    }

    Position position(ObjectNode* aNode) const
    {
        return aNode;
    }

    virtual Data data(Position aPos) const
    {
        TimeKeyBlender::SeekData data = {};
        ObjectNode* node = static_cast<ObjectNode*>(aPos);
        if (node && node->timeLine())
        {
            data.objNode = node;
            data.expans = mUseCache ?
                        &(node->timeLine()->working()) :
                        &(node->timeLine()->current());
            return data;
        }
        return data;
    }

    virtual Position parent(Position aPos) const
    {
        if (!aPos) return nullptr;
        return static_cast<ObjectNode*>(aPos)->parent();
    }

    virtual Position child(Position aPos) const
    {
        ObjectNode* node = static_cast<ObjectNode*>(aPos);
        if (!node || node->children().empty()) return nullptr;
        return node->children().front();
    }

    virtual Position prevSib(Position aPos) const
    {
        ObjectNode* node = static_cast<ObjectNode*>(aPos);
        if (!node) return nullptr;
        return node->prevSib();
    }

    virtual Position nextSib(Position aPos) const
    {
        ObjectNode* node = static_cast<ObjectNode*>(aPos);
        if (!node) return nullptr;
        return node->nextSib();
    }
};

} // namespace core


namespace core
{

//-------------------------------------------------------------------------------------------------
SRTExpans TimeKeyBlender::getSRTExpans(
        ObjectNode& aNode, const TimeInfo& aTime)
{
    if (aNode.timeLine())
    {
        ObjectTreeSeeker seeker(true);
        TimeKeyBlender blender(seeker, seeker.position(&aNode));
        blender.blendSRTKey(seeker.position(&aNode), aTime);
        return aNode.timeLine()->working().srt();
    }
    return SRTExpans();
}

//-------------------------------------------------------------------------------------------------
QMatrix4x4 TimeKeyBlender::getWorldMatrix(
        ObjectNode& aNode, const TimeInfo& aTime)
{
    if (aNode.timeLine())
    {
        ObjectTreeSeeker seeker(true);
        TimeKeyBlender blender(seeker, seeker.position(&aNode));
        blender.blendSRTKey(seeker.position(&aNode), aTime);
        return aNode.timeLine()->working().srt().worldMatrix();
    }
    return QMatrix4x4();
}

QMatrix4x4 TimeKeyBlender::getRelativeMatrix(
        ObjectNode& aNode, const TimeInfo& aTime, const ObjectNode* aParent)
{
    if (aNode.timeLine())
    {
        ObjectTreeSeeker seeker(true);
        TimeKeyBlender blender(seeker, seeker.position(&aNode));
        blender.blendSRTKey(seeker.position(&aNode), aTime);

        QMatrix4x4 result;
        ObjectNode* current = &aNode;
        while (current)
        {
            if (current == aParent)
            {
                return result;
            }

            auto line = current->timeLine();
            if (line)
            {
                result = line->working().srt().data().localMatrix() * result;
            }

            current = current->parent();
        }
    }
    return QMatrix4x4();
}

LayerMesh* TimeKeyBlender::getAreaMesh(
        ObjectNode& aNode, const TimeInfo& aTime)
{
    if (aNode.timeLine())
    {
        auto& map = aNode.timeLine()->map(TimeKeyType_Mesh);
        auto key = (MeshKey*)TimeKeyGatherer::findLastKey(map, aTime.frame);
        if (key) return &(key->data());
    }
    return aNode.gridMesh();
}

//-------------------------------------------------------------------------------------------------
TimeKeyBlender::TimeKeyBlender(ObjectTree& aTree)
    : mSeeker()
    , mRoot()
{
    static ObjectTreeSeeker sSeeker(false);
    mSeeker = &sSeeker;
    mRoot = sSeeker.position(aTree.topNode());
}

TimeKeyBlender::TimeKeyBlender(ObjectNode& aRootNode, bool aUseWorking)
    : mSeeker()
    , mRoot()
{
    if (aUseWorking)
    {
        static ObjectTreeSeeker sSeeker(true);
        mSeeker = &sSeeker;
        mRoot = sSeeker.position(&aRootNode);
    }
    else
    {
        static ObjectTreeSeeker sSeeker(false);
        mSeeker = &sSeeker;
        mRoot = sSeeker.position(&aRootNode);
    }
}

TimeKeyBlender::TimeKeyBlender(SeekerType& aSeeker, PositionType aRoot)
    : mSeeker(&aSeeker)
    , mRoot(aRoot)
{
}

void TimeKeyBlender::updateCurrents(ObjectNode* aRootNode, const TimeInfo& aTime)
{
    {
        util::TreeSeekIterator<SeekData> itr(*mSeeker, mRoot);

        while (itr.hasNext())
        {
            auto pos = itr.next();
            XC_ASSERT(pos);

            // build srt
            blendSRTKey(pos, aTime);

            // build opa
            blendOpaKey(pos, aTime);

            // build bone
            blendBoneKey(pos, aTime);

            // build pose
            blendPoseKey(pos, aTime);

            // build mesh
            blendMeshKey(pos, aTime);

            // build ffd
            blendFFDKey(pos, aTime);
        }
    }

    // build skeletal animation matrix
    if (aRootNode)
    {
        // palette
        PosePalette::KeyPairs pairs;
        buildPosePalette(*aRootNode, pairs);
        // set map
        setBoneInfluenceMaps(*aRootNode, nullptr, aTime);
    }

    // set master cache
    {
        util::TreeSeekIterator<SeekData> itr(*mSeeker, mRoot);

        while (itr.hasNext())
        {
            auto pos = itr.next();
            XC_ASSERT(pos);
            auto expans = mSeeker->data(pos).expans;
            if (!expans) continue;
            expans->setMasterCache(aTime.frame);
        }
    }
}

void TimeKeyBlender::clearCaches(TimeLineEvent& aEvent)
{
    for (TimeLineEvent::Target& target : aEvent.targets())
    {
        target.pos.line()->current().clearCaches();
        target.pos.line()->working().clearCaches();

        // clear master caches of parents
        XC_PTR_ASSERT(target.node);
        auto parent = target.node->parent();
        while (parent)
        {
            auto pline = parent->timeLine();
            if (pline)
            {
                pline->current().clearMasterCache();
                pline->working().clearMasterCache();
            }
            parent = parent->parent();
        }
    }
}

void TimeKeyBlender::clearCaches(ObjectNode* aRootNode)
{
    ObjectNode::Iterator itr(aRootNode);
    while (itr.hasNext())
    {
        ObjectNode* node = itr.next();
        XC_PTR_ASSERT(node);

        if (node->timeLine())
        {
            node->timeLine()->current().clearCaches();
            node->timeLine()->working().clearCaches();
        }
    }

    // clear master caches of parents
    if (aRootNode)
    {
        ObjectNode* parent = aRootNode->parent();
        while (parent)
        {
            auto pline = parent->timeLine();
            if (pline)
            {
                pline->current().clearMasterCache();
                pline->working().clearMasterCache();
            }
            parent = parent->parent();
        }
    }
}

SRTKey::Data TimeKeyBlender::getDefaultSRT(const ObjectNode& aNode)
{
    const QVector3D initialPos = QVector3D(aNode.initialCenter());
    SRTKey::Data data;

    const ObjectNode* parent = aNode.parent();
    while (parent)
    {
        if (parent->timeLine())
        {
            const QVector3D parentPos = QVector3D(parent->initialCenter());
            data.pos = initialPos - parentPos;
            return data;
        }
        parent = parent->parent();
    }

    return data;
}

#if 0
QMatrix4x4 TimeKeyBlender::getParentMatrix(PositionType aPos, int aCacheFrame)
{
    PositionType parent = mSeeker->parent(aPos);
    while (parent)
    {
        auto seekData = mSeeker->data(parent);
        if (seekData.expans)
        {
            return seekData.expans->srt().worldMatrix();
        }
        parent = mSeeker->parent(parent);
    }
    return QMatrix4x4();
}
#endif

void TimeKeyBlender::blendSRTKey(PositionType aPos, const TimeInfo& aTime)
{
    auto seekData = mSeeker->data(aPos);
    if (!seekData.objNode || !seekData.expans) return;
    auto& node = *seekData.objNode;
    auto& expans = *seekData.expans;

    // set srt
    getSRTData(expans, node, aTime);

    // set initial rect and center
    expans.srt().setInitialRect(node.initialRect());
    expans.srt().setInitialCenter(node.initialCenter());

    // update matrix
    expans.srt().parentMatrix() = QMatrix4x4();
    {
        auto pos = mSeeker->parent(aPos);
        while (pos)
        {
            auto data = mSeeker->data(pos);
            if (!data.expans)
            {
                pos = mSeeker->parent(pos);
                continue;
            }
            if (!data.expans->hasKeyCache(TimeKeyType_SRT, aTime.frame))
            {
                blendSRTKey(pos, aTime);
            }
            expans.srt().parentMatrix() = data.expans->srt().worldMatrix();
            break;
        }
    }
}

void TimeKeyBlender::getSRTData(
        TimeKeyExpans& aCurrent, const ObjectNode& aNode, const TimeInfo& aTime)
{
    XC_PTR_ASSERT(aNode.timeLine());
    TimeKeyGatherer blend(aNode.timeLine()->map(TimeKeyType_SRT), aTime);
    auto& srt = aCurrent.srt();
    aCurrent.setKeyCache(TimeKeyType_SRT, aTime.frame);

    if (blend.isEmpty())
    { // no key is exists
        srt.data() = getDefaultSRT(aNode);
    }
    else if (blend.hasSameFrame())
    { // a key is exists
        srt.data() = ((const SRTKey*)blend.point(0).key)->data();
    }
    else if (blend.isSingle())
    { // perfect following
        srt.data() = ((const SRTKey*)blend.singlePoint().key)->data();
    }
    else
    {
        // blend keys
        auto p0 = blend.point(0);
        auto p1 = blend.point(1);
        const SRTKey* k0 = (const SRTKey*)p0.key;
        const SRTKey* k1 = (const SRTKey*)p1.key;
        const float frame = p1.relativeFrame - p0.relativeFrame;
        XC_ASSERT(frame != 0.0f);

        srt.data().easing = k0->data().easing;
        srt.data().spline = k0->data().spline;

        // calculate easing
        const float time = util::Easing::calculate(
                    srt.data().easing, -p0.relativeFrame, 0.0f, 1.0f, frame);

        // use spline cache
        if (srt.hasSplineCache(aTime.frame))
        {
            srt.data().pos = srt.spline().getByLinear(time);
            srt.data().rotate = k0->rotate() * (1.0f - time) + k1->rotate() * time;
            srt.data().scale = k0->scale() * (1.0f - time) + k1->scale() * time;
        }
        else
        {
            // linear blending
            const std::array<QVector3D, 2> vels = catmullRomVels(blend);
            srt.spline().set(k0->pos(), k1->pos(), vels[0], vels[1]);
            srt.setSplineCache(util::Range(p0.frame, p1.frame));

            srt.data().pos = srt.spline().getByLinear(time);
            srt.data().rotate = k0->rotate() * (1.0f - time) + k1->rotate() * time;
            srt.data().scale = k0->scale() * (1.0f - time) + k1->scale() * time;
        }
    }
}

void TimeKeyBlender::blendOpaKey(PositionType aPos, const TimeInfo& aTime)
{
    auto seekData = mSeeker->data(aPos);
    if (!seekData.objNode || !seekData.expans) return;
    auto& node = *seekData.objNode;
    auto& expans = *seekData.expans;
    if (!node.timeLine()) return;

    // set cache frame
    expans.setKeyCache(TimeKeyType_Opa, aTime.frame);

    // get blend info
    TimeKeyGatherer blend(node.timeLine()->map(TimeKeyType_Opa), aTime);

    // no key is exists
    if (blend.isEmpty())
    {
        expans.opa() = OpaKey::Data();
    }
    else if (blend.hasSameFrame())
    {
        // a key is exists
        expans.opa() = ((const OpaKey*)blend.point(0).key)->data();
    }
    else if (blend.isSingle())
    {
        // perfect following
        expans.opa() = ((const OpaKey*)blend.singlePoint().key)->data();
    }
    else
    {
        // blend keys
        auto p0 = blend.point(0);
        auto p1 = blend.point(1);
        auto key0 = (const OpaKey*)p0.key;
        auto key1 = (const OpaKey*)p1.key;
        const float frame = p1.relativeFrame - p0.relativeFrame;
        XC_ASSERT(frame != 0.0f);

        // calculate easing
        expans.opa().easing = key0->data().easing;
        const float time = util::Easing::calculate(
                    expans.opa().easing, -p0.relativeFrame, 0.0f, 1.0f, frame);

        // blend
        expans.opa().opacity =
                key0->data().opacity * (1.0f - time) +
                key1->data().opacity * time;
    }

    // multiply opacity of parents
    {
        expans.setWorldOpacity(expans.opa().opacity);

        auto ppos = mSeeker->parent(aPos);
        while (ppos)
        {
            // find a parent
            auto pdata = mSeeker->data(ppos);
            if (!pdata.expans)
            {
                ppos = mSeeker->parent(ppos);
                continue;
            }

            // check cache
            if (!pdata.expans->hasKeyCache(TimeKeyType_Opa, aTime.frame))
            {
                blendOpaKey(ppos, aTime);
            }

            // world opacity
            expans.setWorldOpacity(
                        pdata.expans->worldOpacity() * expans.opa().opacity);
            break;
        }
    }
}

void TimeKeyBlender::blendBoneKey(PositionType aPos, const TimeInfo& aTime)
{
    auto seekData = mSeeker->data(aPos);
    if (!seekData.objNode || !seekData.expans) return;
    auto& node = *seekData.objNode;
    auto& expans = *seekData.expans;
    if (!node.timeLine()) return;

    const TimeLine::MapType& map = node.timeLine()->map(TimeKeyType_Bone);
    expans.setAreaBone((BoneKey*)TimeKeyGatherer::findLastKey(map, aTime.frame));
}

void TimeKeyBlender::blendPoseKey(PositionType aPos, const TimeInfo& aTime)
{
    auto seekData = mSeeker->data(aPos);
    if (!seekData.objNode || !seekData.expans) return;
    auto& node = *seekData.objNode;
    auto& expans = *seekData.expans;
    if (!node.timeLine()) return;

    // area bone
    auto areaBoneKey = expans.areaBone();
    expans.setPoseParent(areaBoneKey);

    // get blend info
    TimeKeyGatherer blend(
                node.timeLine()->map(TimeKeyType_Pose), aTime,
                TimeKeyGatherer::ForceType_AssignedParent, areaBoneKey);

    // no key is exists
    if (blend.isEmpty())
    {
        expans.pose() = PoseKey::Data();
        expans.setPoseParent(nullptr);
    }
    else if (blend.hasSameFrame())
    {
        // a key is exists
        auto poseKey = (const PoseKey*)(blend.point(0).key);
        expans.pose() = poseKey->data();

        XC_ASSERT(poseKey->parent() &&
                  poseKey->parent() == (TimeKey*)areaBoneKey &&
                  poseKey->parent()->type() == TimeKeyType_Bone);
    }
    else if (blend.isSingle())
    {
        // perfect following
        auto poseKey = (const PoseKey*)(blend.singlePoint().key);
        expans.pose() = poseKey->data();

        XC_ASSERT(poseKey->parent() &&
                  poseKey->parent() == (TimeKey*)areaBoneKey &&
                  poseKey->parent()->type() == TimeKeyType_Bone);
    }
    else
    {
        // blend keys
        auto p0 = blend.point(0);
        auto p1 = blend.point(1);
        auto key0 = (const PoseKey*)p0.key;
        auto key1 = (const PoseKey*)p1.key;
        XC_PTR_ASSERT(key0->parent());
        XC_PTR_ASSERT(key1->parent());
        XC_ASSERT(key0->parent() == key1->parent());
        const float frame = p1.relativeFrame - p0.relativeFrame;
        XC_ASSERT(frame != 0.0f);

        XC_ASSERT(key0->parent() &&
                  key0->parent() == (TimeKey*)areaBoneKey &&
                  key0->parent()->type() == TimeKeyType_Bone);

        // initialize
        expans.pose() = key0->data();

        // calculate easing
        const float time = util::Easing::calculate(
                    expans.pose().easing(), -p0.relativeFrame, 0.0f, 1.0f, frame);

        // blend
        int index = 0;
        for (Bone2* bone : expans.pose().topBones())
        {
            const Bone2* bone0 = key0->data().topBones().at(index);
            const Bone2* bone1 = key1->data().topBones().at(index);

            Bone2::Iterator itr(bone);
            Bone2::ConstIterator itr0(bone0);
            Bone2::ConstIterator itr1(bone1);

            while (itr.hasNext())
            {
                XC_ASSERT(itr0.hasNext());
                XC_ASSERT(itr1.hasNext());

                const float rotate =
                        itr0.next()->rotate() * (1.0f - time) +
                        itr1.next()->rotate() * time;

                auto target = itr.next();
                target->setRotate(rotate);
                target->updateWorldTransform();
            }
            ++index;
        }
    }

}

void TimeKeyBlender::blendMeshKey(PositionType aPos, const TimeInfo& aTime)
{
    auto seekData = mSeeker->data(aPos);
    if (!seekData.objNode || !seekData.expans) return;
    auto& node = *seekData.objNode;
    auto& expans = *seekData.expans;
    if (!node.timeLine()) return;

    const TimeLine::MapType& map = node.timeLine()->map(TimeKeyType_Mesh);
    expans.setAreaMesh((MeshKey*)TimeKeyGatherer::findLastKey(map, aTime.frame));
}

MeshKey* TimeKeyBlender::getMeshKey(const ObjectNode& aNode, const TimeInfo& aTime)
{
    if (!aNode.timeLine()) return nullptr;
    const TimeLine::MapType& map = aNode.timeLine()->map(TimeKeyType_Mesh);
    return (MeshKey*)TimeKeyGatherer::findLastKey(map, aTime.frame);
}

void TimeKeyBlender::blendFFDKey(PositionType aPos, const TimeInfo& aTime)
{
    auto seekData = mSeeker->data(aPos);
    if (!seekData.objNode || !seekData.expans) return;
    auto& node = *seekData.objNode;
    auto& expans = *seekData.expans;
    if (!node.timeLine()) return;

    MeshKey* areaMeshKey = getMeshKey(node, aTime);

    // get blend info
    TimeKeyGatherer blend(
                node.timeLine()->map(TimeKeyType_FFD), aTime,
                TimeKeyGatherer::ForceType_AssignedParent, areaMeshKey);

    // find parent's mesh
    const LayerMesh* mesh = node.gridMesh();
    if (areaMeshKey) mesh = &(areaMeshKey->data());

    if (!mesh || mesh->vertexCount() == 0)
    {
        expans.ffd().clear();
        return;
    }

    // no key is exists
    if (blend.isEmpty())
    {
        expans.ffd().write(mesh->positions(), mesh->vertexCount());
    }
    else if (blend.hasSameFrame())
    {
        // a key is exists
        XC_ASSERT(blend.point(0).key->parent() == (TimeKey*)areaMeshKey);
        expans.ffd().write(((const FFDKey*)(blend.point(0).key))->data().positions(), mesh->vertexCount());
    }
    else if (blend.isSingle())
    {
        // perfect following
        XC_ASSERT(blend.singlePoint().key->parent() == (TimeKey*)areaMeshKey);
        expans.ffd().write(((const FFDKey*)(blend.singlePoint().key))->data().positions(), mesh->vertexCount());
    }
    else
    {
        // blend keys
        auto p0 = blend.point(0);
        auto p1 = blend.point(1);
        auto key0 = (const FFDKey*)p0.key;
        auto key1 = (const FFDKey*)p1.key;
        const gl::Vector3* v0 = key0->data().positions();
        const gl::Vector3* v1 = key1->data().positions();
        const int count = key0->data().count();
        XC_ASSERT(count == key1->data().count());
        XC_ASSERT(key0->parent() == (TimeKey*)areaMeshKey);
        XC_ASSERT(key1->parent() == (TimeKey*)areaMeshKey);
        const float frame = p1.relativeFrame - p0.relativeFrame;
        XC_ASSERT(frame != 0.0f);

        // alloc if need
        expans.ffd().alloc(count);

        // calculate easing
        expans.ffd().easing() = key0->data().easing();
        const float time = util::Easing::calculate(
                    expans.ffd().easing(), -p0.relativeFrame, 0.0f, 1.0f, frame);

        // linear blend
        gl::Vector3* dst = expans.ffd().positions();
        for (int i = 0; i < count; ++i)
        {
            dst[i] = v0[i] * (1.0f - time) + v1[i] * time;
        }
    }
}

void TimeKeyBlender::buildPosePalette(ObjectNode& aNode, PosePalette::KeyPairs& aPairs)
{
    bool pushed = false;
    if (aNode.timeLine())
    {
        XC_PTR_ASSERT(mSeeker->data(&aNode).expans);
        auto& expans = *(mSeeker->data(&aNode).expans);

        // update pairs
        if (expans.poseParent())
        {
            PosePalette::KeyPair pair = {
                &expans.poseParent()->data(), &expans.pose()
            };
            aPairs.push_back(pair);
            pushed = true;
        }

        // build
        expans.posePalette().build(aPairs);
    }

    // iterate children
    for (auto child : aNode.children())
    {
        XC_PTR_ASSERT(child);
        buildPosePalette(*child, aPairs);
    }

    if (pushed)
    {
        aPairs.pop_back();
    }
}

void TimeKeyBlender::setBoneInfluenceMaps(
        ObjectNode& aNode, const BoneKey* aKey, const TimeInfo& aTime)
{
    const BoneKey* key = aKey;

    if (aNode.timeLine())
    {
        XC_PTR_ASSERT(mSeeker->data(&aNode).expans);
        auto& expans = *(mSeeker->data(&aNode).expans);

        auto parent = expans.poseParent();
        if (parent) key = parent;

        const LayerMesh* mesh = nullptr;
        BoneInfluenceMap* influence = nullptr;
        QMatrix4x4 outerMtx;
        QMatrix4x4 innerMtx;
        const bool isDefaultMesh = true;//!expans.areaMesh();

        if (key && isDefaultMesh)
        {
            for (auto cache : key->caches())
            {
                if (cache->node() == static_cast<const ObjectNode*>(&aNode))
                {
                    //qDebug() << "infl" << cache->node() << (&aNode) << cache->influence().vertexCount();
                    mesh = TimeKeyBlender::getAreaMesh(aNode, aTime);
                    XC_ASSERT(cache->frameSign() == mesh->frameSign());

                    influence = &cache->influence();
                    innerMtx = cache->innerMatrix();
                    break;
                }
            }

            auto cacheOwner = key->cacheOwner();
            if (cacheOwner)
            {
                auto ownerExpans = mSeeker->data(cacheOwner).expans;
                if (ownerExpans)
                {
                    outerMtx = ownerExpans->srt().worldMatrix();
                }
            }
        }
        expans.setBoneParent(mesh);
        expans.setBoneInfluence(influence);
        expans.setOuterMatrix(outerMtx);
        expans.setInnerMatrix(innerMtx);
    }

    for (auto child : aNode.children())
    {
        setBoneInfluenceMaps(*child, key, aTime);
    }
}

std::array<QVector3D, 2> TimeKeyBlender::catmullRomVels(const TimeKeyGatherer& aBlend)
{
    const SRTKey* k0 = (const SRTKey*)aBlend.point(-1).key;
    const SRTKey* k1 = (const SRTKey*)aBlend.point( 0).key;
    const SRTKey* k2 = (const SRTKey*)aBlend.point( 1).key;
    const SRTKey* k3 = (const SRTKey*)aBlend.point( 2).key;
    std::array<QVector3D, 2> result;

    if (k1->data().spline == SRTKey::SplineType_Linear)
    {
        k0 = nullptr;
    }
    if (k2->data().spline == SRTKey::SplineType_Linear)
    {
        k3 = nullptr;
    }

    if (!k0)
    {
        const QVector3D linear = k2->pos() - k1->pos();

        if (!k3)
        {
            result[1] = linear;
            result[0]  = linear;
        }
        else
        {
            result[1] = 0.5f * (k3->pos() - k1->pos());
            result[0]  = util::MathUtil::getAxisInversed(linear.normalized(), result[1]);
        }
    }
    else
    {
        if (!k3)
        {
            const QVector3D linear = k2->pos() - k1->pos();

            result[0]  = 0.5f * (k2->pos() - k0->pos());
            result[1] = util::MathUtil::getAxisInversed(linear.normalized(), result[0]);
        }
        else
        {
            result[0]  = 0.5f * (k2->pos() - k0->pos());
            result[1] = 0.5f * (k3->pos() - k1->pos());
        }
    }
    return result;
}

} // namespace core

