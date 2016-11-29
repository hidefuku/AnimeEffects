#include "util/TreeIterator.h"
#include "util/TreeSeekIterator.h"
#include "util/MathUtil.h"
#include "core/TimeKeyExpans.h"
#include "core/TimeKeyBlender.h"
#include "core/LayerMesh.h"
#include "core/ObjectNodeUtil.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
class ObjectTreeSeeker : public util::ITreeSeeker<TimeKeyBlender::SeekData, ObjectNode*>
{
    bool mUseCache;
public:
    ObjectTreeSeeker(bool aUseCache)
        : mUseCache(aUseCache)
    {
    }

    virtual Position position(ObjectNode* aNode) const
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
QMatrix4x4 TimeKeyBlender::getLocalSRMatrix(
        const ObjectNode& aNode, const TimeInfo& aTime)
{
    if (aNode.timeLine())
    {
        SRTExpans expans;
        getRotateExpans(expans, aNode, aTime);
        getScaleExpans(expans, aNode, aTime);
        return expans.localSRMatrix();
    }
    return QMatrix4x4();
}

//-------------------------------------------------------------------------------------------------
QMatrix4x4 TimeKeyBlender::getWorldMatrix(
        ObjectNode& aNode, const TimeInfo& aTime)
{
    if (aNode.timeLine())
    {
        ObjectTreeSeeker seeker(true);
        TimeKeyBlender blender(seeker, seeker.position(&aNode));
        blender.blendSRTKeys(seeker.position(&aNode), aTime);
        return aNode.timeLine()->working().srt().worldMatrix();
    }
    return QMatrix4x4();
}

QMatrix4x4 TimeKeyBlender::getRelativeMatrix(
        ObjectNode& aNode, const TimeInfo& aTime, const ObjectNode* aParent)
{
    QMatrix4x4 result;

    if (aNode.timeLine())
    {
        ObjectTreeSeeker seeker(true);
        TimeKeyBlender blender(seeker, seeker.position(&aNode));
        blender.blendSRTKeys(seeker.position(&aNode), aTime);

        // calculate matrix
        for (auto curr = &aNode; curr && curr != aParent; curr = curr->parent())
        {
            auto currLine = curr->timeLine();
            if (currLine)
            {
                result = currLine->working().srt().localMatrix() * result;
            }
        }
    }
    return result;
}

LayerMesh* TimeKeyBlender::getAreaMesh(ObjectNode& aNode, const TimeInfo& aTime)
{
    return getAreaMeshImpl(aNode, aTime).second;
}

std::pair<TimeKey*, LayerMesh*> TimeKeyBlender::getAreaMeshImpl(ObjectNode& aNode, const TimeInfo& aTime)
{
    if (aNode.timeLine())
    {
        // find parent key
        ImageKey* imageKey = getImageKey(aNode, aTime);
        MeshKey* meshKey = getMeshKey(aNode, aTime);

        if (imageKey && (!meshKey || imageKey->frame() > meshKey->frame()))
        {
            return std::pair<TimeKey*, LayerMesh*>(imageKey, &(imageKey->data().gridMesh()));
        }
        else if (meshKey)
        {
            return std::pair<TimeKey*, LayerMesh*>(meshKey, &(meshKey->data()));
        }
    }
    return std::pair<TimeKey*, LayerMesh*>(nullptr, nullptr);
}

BoneKey* TimeKeyBlender::getAreaBone(
        ObjectNode& aNode, const TimeInfo& aTime)
{
    if (aNode.timeLine())
    {
        auto& map = aNode.timeLine()->map(TimeKeyType_Bone);
        TimeKey* key = TimeKeyGatherer::findLastKey(map, aTime.frame);
        if (key)
        {
            TIMEKEY_PTR_TYPE_ASSERT(key, Bone);
            return (BoneKey*)key;
        }
    }
    return nullptr;
}

BoneKey* TimeKeyBlender::getNearestInfluencerBone(
        ObjectNode& aNode, const TimeInfo& aTime)
{
    for (ObjectNode* node = &aNode; node; node = node->parent())
    {
        auto areaBone = getAreaBone(aNode, aTime);
        if (areaBone) return areaBone;
    }
    return nullptr;
}

QVector2D TimeKeyBlender::getImageOffset(ObjectNode& aNode, const TimeInfo& aTime)
{
    auto imageKey = getImageKey(aNode, aTime);
    return imageKey ? imageKey->data().imageOffset() : QVector2D();
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
        util::TreeSeekIterator<SeekData, ObjectNode*> itr(*mSeeker, mRoot);

        while (itr.hasNext())
        {
            auto pos = itr.next();
            XC_ASSERT(pos);

            // build move, rotate and scale
            blendSRTKeys(pos, aTime);

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

            // build image
            blendImageKey(pos, aTime);
        }
    }

    // build skeletal animation matrix
    if (aRootNode)
    {
        // palette
#if 0
        PosePalette::KeyPairs pairs;
        buildPosePalette(*aRootNode, pairs);
#else
        PosePalette::KeyPair pair = {};
        buildPosePalette(*aRootNode, pair);
#endif
        // set map
        setBoneInfluenceMaps(*aRootNode, nullptr, aTime);
        // set binder
        setBinderBones(*aRootNode);
    }

    // set master cache
    {
        util::TreeSeekIterator<SeekData, ObjectNode*> itr(*mSeeker, mRoot);

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

template<class tKey>
float getEasingRateFromTwoKeys(const TimeKeyGatherer& aGatherer)
{
    // blend keys
    auto p0 = aGatherer.point(0);
    auto p1 = aGatherer.point(1);
    XC_ASSERT(p0 && p1);
    const float frame = p1.relativeFrame - p0.relativeFrame;
    XC_ASSERT(frame != 0.0f);

    const tKey* k0 = (const tKey*)p0.key;

    // calculate easing
    return util::Easing::calculate(
                k0->data().easing(), -p0.relativeFrame, 0.0f, 1.0f, frame);
}

template<class tKey, TimeKeyType tType>
typename tKey::Data getDefaultKeyData(const ObjectNode& aNode)
{
    if (aNode.timeLine())
    {
        auto key = (tKey*)aNode.timeLine()->defaultKey(tType);
        if (key) return key->data();
    }
    return tKey::Data();
}

void TimeKeyBlender::getMoveExpans(SRTExpans& aExpans, const ObjectNode& aNode, const TimeInfo& aTime)
{
    XC_ASSERT(aNode.timeLine());
    TimeKeyGatherer blend(aNode.timeLine()->map(TimeKeyType_Move), aTime);

    if (blend.isEmpty())
    { // no key is exists
        aExpans.setPos(getDefaultKeyData<MoveKey, TimeKeyType_Move>(aNode).pos());
    }
    else if (blend.hasSameFrame())
    { // a key is exists
        aExpans.setPos(((const MoveKey*)blend.point(0).key)->data().pos());
    }
    else if (blend.isSingle())
    { // perfect following
        aExpans.setPos(((const MoveKey*)blend.singlePoint().key)->data().pos());
    }
    else
    {
        // blend keys
        auto p0 = blend.point(0);
        auto p1 = blend.point(1);
        const MoveKey* k0 = (const MoveKey*)p0.key;
        const MoveKey* k1 = (const MoveKey*)p1.key;
        const MoveKey* kn = (const MoveKey*)blend.point(-1).key;
        const MoveKey* k2 = (const MoveKey*)blend.point(2).key;

        // calculate easing
        const float time = getEasingRateFromTwoKeys<MoveKey>(blend);

        // use spline cache
        if (aExpans.hasSplineCache(aTime.frame))
        {
            aExpans.setPos(aExpans.spline().getByLinear(time).toVector2D());
        }
        else
        {
            // linear blending
            auto vels = MoveKey::getCatmullRomVels(kn, k0, k1, k2);
            aExpans.spline().set(k0->pos(), k1->pos(), vels[0], vels[1]);
            aExpans.setSplineCache(util::Range(p0.frame, p1.frame));

            aExpans.setPos(aExpans.spline().getByLinear(time).toVector2D());
        }
    }
}

void TimeKeyBlender::getRotateExpans(SRTExpans& aExpans, const ObjectNode& aNode, const TimeInfo& aTime)
{
    XC_ASSERT(aNode.timeLine());
    TimeKeyGatherer blend(aNode.timeLine()->map(TimeKeyType_Rotate), aTime);

    if (blend.isEmpty())
    { // no key is exists
        aExpans.setRotate(getDefaultKeyData<RotateKey, TimeKeyType_Rotate>(aNode).rotate());
    }
    else if (blend.hasSameFrame())
    { // a key is exists
        aExpans.setRotate(((const RotateKey*)blend.point(0).key)->data().rotate());
    }
    else if (blend.isSingle())
    { // perfect following
        aExpans.setRotate(((const RotateKey*)blend.singlePoint().key)->data().rotate());
    }
    else
    {
        const RotateKey* k0 = (const RotateKey*)blend.point(0).key;
        const RotateKey* k1 = (const RotateKey*)blend.point(1).key;
        // calculate easing
        const float time = getEasingRateFromTwoKeys<RotateKey>(blend);
        aExpans.setRotate(k0->rotate() * (1.0f - time) + k1->rotate() * time);
    }
}

void TimeKeyBlender::getScaleExpans(SRTExpans& aExpans, const ObjectNode& aNode, const TimeInfo& aTime)
{
    XC_ASSERT(aNode.timeLine());
    TimeKeyGatherer blend(aNode.timeLine()->map(TimeKeyType_Scale), aTime);

    if (blend.isEmpty())
    { // no key is exists
        aExpans.setScale(getDefaultKeyData<ScaleKey, TimeKeyType_Scale>(aNode).scale());
    }
    else if (blend.hasSameFrame())
    { // a key is exists
        aExpans.setScale(((const ScaleKey*)blend.point(0).key)->data().scale());
    }
    else if (blend.isSingle())
    { // perfect following
        aExpans.setScale(((const ScaleKey*)blend.singlePoint().key)->data().scale());
    }
    else
    {
        const ScaleKey* k0 = (const ScaleKey*)blend.point(0).key;
        const ScaleKey* k1 = (const ScaleKey*)blend.point(1).key;
        // calculate easing
        const float time = getEasingRateFromTwoKeys<ScaleKey>(blend);
        aExpans.setScale(k0->scale() * (1.0f - time) + k1->scale() * time);
    }
}

void TimeKeyBlender::blendSRTKeys(PositionType aPos, const TimeInfo& aTime)
{
    auto seekData = mSeeker->data(aPos);
    if (!seekData.objNode || !seekData.expans) return;
    auto& node = *seekData.objNode;
    auto& expans = *seekData.expans;
    if (!node.timeLine()) return;
    auto frame = aTime.frame;

    expans.setKeyCache(TimeKeyType_Move, frame);
    getMoveExpans(expans.srt(), node, aTime);

    expans.setKeyCache(TimeKeyType_Rotate, frame);
    getRotateExpans(expans.srt(), node, aTime);

    expans.setKeyCache(TimeKeyType_Scale, frame);
    getScaleExpans(expans.srt(), node, aTime);

    // update matrix
    expans.srt().setParentMatrix(QMatrix4x4());
    {
        auto ppos = mSeeker->parent(aPos);
        while (ppos)
        {
            auto pdata = mSeeker->data(ppos);
            if (!pdata.expans)
            {
                ppos = mSeeker->parent(ppos);
                continue;
            }

            auto hasMoveCache = pdata.expans->hasKeyCache(TimeKeyType_Move, frame);
            auto hasRotateCache = pdata.expans->hasKeyCache(TimeKeyType_Rotate, frame);
            auto hasScaleCache = pdata.expans->hasKeyCache(TimeKeyType_Scale, frame);

            if (!hasMoveCache || !hasRotateCache || !hasScaleCache)
            {
                blendSRTKeys(ppos, aTime);
            }

            expans.srt().setParentMatrix(pdata.expans->srt().worldMatrix());
            break;
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
        auto defaultKey = (OpaKey*)node.timeLine()->defaultKey(TimeKeyType_Opa);
        expans.opa() = defaultKey ? defaultKey->data() : OpaKey::Data();
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
        const OpaKey* k0 = (const OpaKey*)blend.point(0).key;
        const OpaKey* k1 = (const OpaKey*)blend.point(1).key;
        // calculate easing
        const float time = getEasingRateFromTwoKeys<OpaKey>(blend);
        // blend
        expans.opa().setOpacity(k0->opacity() * (1.0f - time) + k1->opacity() * time);
    }

    // multiply opacity of parents
    {
        expans.setWorldOpacity(expans.opa().opacity());

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
                        pdata.expans->worldOpacity() * expans.opa().opacity());
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

    expans.bone().setAreaKey(getAreaBone(node, aTime));
}

void TimeKeyBlender::blendPoseKey(PositionType aPos, const TimeInfo& aTime)
{
    auto seekData = mSeeker->data(aPos);
    if (!seekData.objNode || !seekData.expans) return;
    auto& node = *seekData.objNode;
    auto& expans = *seekData.expans;
    if (!node.timeLine()) return;

    // area bone
    auto areaBoneKey = expans.bone().areaKey();
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

        TIMEKEY_PTR_TYPE_ASSERT(poseKey->parent(), Bone);
        XC_ASSERT(poseKey->parent() == (TimeKey*)areaBoneKey);
    }
    else if (blend.isSingle())
    {
        // perfect following
        auto poseKey = (const PoseKey*)(blend.singlePoint().key);
        expans.pose() = poseKey->data();

        TIMEKEY_PTR_TYPE_ASSERT(poseKey->parent(), Bone);
        XC_ASSERT(poseKey->parent() == (TimeKey*)areaBoneKey);
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
        TIMEKEY_PTR_TYPE_ASSERT(key0->parent(), Bone);
        XC_ASSERT(key0->parent() == (TimeKey*)areaBoneKey);

        // initialize
        expans.pose() = key0->data();

        // calculate easing
        const float time = getEasingRateFromTwoKeys<PoseKey>(blend);

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

    expans.setAreaMeshKey(getMeshKey(node, aTime));
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

    // find area key and mesh
    auto areaMeshPair = getAreaMeshImpl(node, aTime);
    TimeKey* areaKey = areaMeshPair.first;
    LayerMesh* areaMesh = areaMeshPair.second;
    expans.setFFDMesh(areaMesh);
    expans.setFFDMeshParent(areaKey);

    // no mesh exists
    if (!areaMesh || areaMesh->vertexCount() == 0)
    {
        expans.ffd().clear();
        return;
    }

    // get blend info
    TimeKeyGatherer blend(
                node.timeLine()->map(TimeKeyType_FFD), aTime,
                TimeKeyGatherer::ForceType_AssignedParent, areaKey);

    // no key is exists
    if (blend.isEmpty())
    {
        expans.ffd().write(areaMesh->positions(), areaMesh->vertexCount());
    }
    else if (blend.hasSameFrame())
    {
        // a key is exists
        XC_ASSERT(blend.point(0).key->parent() == areaKey);
        expans.ffd().write(((const FFDKey*)(blend.point(0).key))->data().positions(), areaMesh->vertexCount());
    }
    else if (blend.isSingle())
    {
        // perfect following
        XC_ASSERT(blend.singlePoint().key->parent() == areaKey);
        expans.ffd().write(((const FFDKey*)(blend.singlePoint().key))->data().positions(), areaMesh->vertexCount());
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
        XC_ASSERT(key0->parent() == areaKey);
        XC_ASSERT(key1->parent() == areaKey);

        // alloc if need
        expans.ffd().alloc(count);

        // calculate easing
        expans.ffd().easing() = key0->data().easing();
        const float time = getEasingRateFromTwoKeys<FFDKey>(blend);

        // linear blend
        gl::Vector3* dst = expans.ffd().positions();
        for (int i = 0; i < count; ++i)
        {
            dst[i] = v0[i] * (1.0f - time) + v1[i] * time;
        }
    }
}

void TimeKeyBlender::blendImageKey(PositionType aPos, const TimeInfo& aTime)
{
    auto seekData = mSeeker->data(aPos);
    if (!seekData.objNode || !seekData.expans) return;
    auto& node = *seekData.objNode;
    auto& expans = *seekData.expans;
    if (!node.timeLine()) return;

    auto imageKey = getImageKey(node, aTime);
    expans.setAreaImageKey(imageKey);
    expans.setImageOffset(imageKey ? imageKey->data().imageOffset() : QVector2D());
}

ImageKey* TimeKeyBlender::getImageKey(const ObjectNode& aNode, const TimeInfo& aTime)
{
    if (!aNode.timeLine()) return nullptr;
    const TimeLine::MapType& map = aNode.timeLine()->map(TimeKeyType_Image);
    auto imageKey = (ImageKey*)TimeKeyGatherer::findLastKey(map, aTime.frame);
    return imageKey ? imageKey : (ImageKey*)aNode.timeLine()->defaultKey(TimeKeyType_Image);
}

void TimeKeyBlender::buildPosePalette(ObjectNode& aNode, PosePalette::KeyPair aPair)
{
    if (aNode.timeLine())
    {
        XC_PTR_ASSERT(mSeeker->data(&aNode).expans);
        auto& expans = *(mSeeker->data(&aNode).expans);

        if (expans.poseParent())
        {
            PosePalette::KeyPair pair = { &expans.poseParent()->data(), &expans.pose() };
            aPair = pair;
        }
        else if (expans.bone().areaKey())
        {
            PosePalette::KeyPair pair = {};
            aPair = pair;
        }

        // build
        if (aPair.origin && aPair.pose)
        {
            PosePalette::KeyPairs pairs;
            pairs.push_back(aPair);
            expans.posePalette().build(pairs);
        }
        else
        {
            expans.posePalette().clear();
        }
    }

    // iterate children
    for (auto child : aNode.children())
    {
        XC_PTR_ASSERT(child);
        buildPosePalette(*child, aPair);
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

        auto parent = expans.bone().areaKey();
        if (parent) key = parent;

        const LayerMesh* mesh = nullptr;
        BoneInfluenceMap* influence = nullptr;
        QMatrix4x4 outerMtx;
        QMatrix4x4 innerMtx;
        QVector2D imageOffset = expans.imageOffset();

        if (key)
        {
            auto cache = key->findCache(aNode);
            if (cache)
            {
                //qDebug() << "infl" << cache->node() << (&aNode) << cache->influence().vertexCount();
                mesh = TimeKeyBlender::getAreaMesh(aNode, aTime);
                XC_ASSERT(cache->frameSign() == mesh->frameSign());

                influence = &cache->influence();
                innerMtx = cache->innerMatrix();
                imageOffset = cache->imageOffset();
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
        expans.bone().setTargetMesh(mesh);
        expans.bone().setInfluenceMap(influence);
        expans.bone().setOuterMatrix(outerMtx);
        expans.bone().setInnerMatrix(innerMtx);
        expans.setImageOffset(imageOffset);
    }

    for (auto child : aNode.children())
    {
        setBoneInfluenceMaps(*child, key, aTime);
    }
}

void TimeKeyBlender::setBinderBones(ObjectNode& aRootNode)
{
    // reset binding references
    {
        ObjectNode::Iterator itr(&aRootNode);
        while (itr.hasNext())
        {
            auto seekData = mSeeker->data(mSeeker->position(itr.next()));
            if (!seekData.expans) continue;
            seekData.expans->bone().setBinderIndex(-1);
        }
    }
    // update binding references
    {
        ObjectNode::Iterator itr(&aRootNode);
        while (itr.hasNext())
        {
            auto node = itr.next();
            auto seekData = mSeeker->data(mSeeker->position(node));
            if (!seekData.objNode || !seekData.expans) continue;

            auto boneKey = seekData.expans->bone().areaKey();
            if (!boneKey) continue;

            for (auto bindingCache : boneKey->bindingCaches())
            {
                auto bound = mSeeker->data(mSeeker->position(bindingCache.node));
                if (bound.expans)
                {
                    bound.expans->bone().setBinderIndex(bindingCache.boneIndex);
                    bound.expans->bone().setBindingRoot(node);
                    bound.expans->bone().setBindingMatrix(bindingCache.innerMtx);
                }
            }
        }
    }

    // set binding parameters
    {
        auto root = mSeeker->data(mSeeker->position(&aRootNode));
        if (root.expans)
        {
            setBindingMatrices(aRootNode,
                               root.expans->bone().isAffectedByBinding(),
                               root.expans->bone().isUnderOfBinding(),
                               root.expans->bone().outerMatrix());
        }
        else
        {
            setBindingMatrices(aRootNode, false, false, QMatrix4x4());
        }
    }
}

void TimeKeyBlender::setBindingMatrices(
        ObjectNode& aNode, bool aAffectedByBinding, bool aUnderOfBinding, QMatrix4x4 aBindingMtx)
{
    auto seekData = mSeeker->data(mSeeker->position(&aNode));
    if (seekData.objNode && seekData.expans)
    {
        auto& expans = *seekData.expans;

        if (aUnderOfBinding)
        {
            if (aAffectedByBinding)
            {
                aBindingMtx = aBindingMtx * expans.srt().localMatrix();
                expans.bone().setOuterMatrix(aBindingMtx);
                QMatrix4x4 innerMtx;
                expans.bone().setInnerMatrix(innerMtx);
            }
            else
            {
                expans.bone().setOuterMatrix(aBindingMtx);
            }
        }

        if (expans.bone().isBoundByBone())
        {
            //qDebug() << "bound by bone";
            aAffectedByBinding = true;
            aUnderOfBinding = true;
            XC_PTR_ASSERT(expans.bone().bindingRoot());
            auto root = mSeeker->data(mSeeker->position(expans.bone().bindingRoot()));
            if (root.expans)
            {
                auto boneIndex = expans.bone().binderIndex();
                XC_ASSERT(boneIndex >= 0);
                if (boneIndex < PosePalette::kMaxCount)
                {
                    auto transform = root.expans->posePalette().matrices()[boneIndex];
                    aBindingMtx = root.expans->bone().outerMatrix() * transform * expans.bone().bindingMatrix();
                    expans.bone().setOuterMatrix(aBindingMtx);
                    expans.bone().setInnerMatrix(QMatrix4x4());
                    //qDebug() << "transform bound by bone";
                }
            }
        }

        expans.bone().setIsUnderOfBinding(aUnderOfBinding);
        expans.bone().setIsAffectedByBinding(aAffectedByBinding);

        if (expans.bone().areaKey())
        {
            aAffectedByBinding = false;
            aBindingMtx = expans.bone().outerMatrix();
        }
    }

    // recursive call
    for (auto child : aNode.children())
    {
        setBindingMatrices(*child, aAffectedByBinding, aUnderOfBinding, aBindingMtx);
    }
}

} // namespace core

