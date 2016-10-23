#include "core/TimeKeyExpans.h"
#include "ctrl/TimeLineUtil.h"
#include "gui/prop/prop_KeyAccessor.h"

namespace
{

const core::SRTKey::Data& getSRTKeyData(const core::ObjectNode& aTarget, int aFrame)
{
    auto key = aTarget.timeLine()->timeKey(core::TimeKeyType_SRT, aFrame);
    XC_PTR_ASSERT(key);
    return ((const core::SRTKey*)key)->data();
}

const core::OpaKey::Data& getOpaKeyData(const core::ObjectNode& aTarget, int aFrame)
{
    auto key = aTarget.timeLine()->timeKey(core::TimeKeyType_Opa, aFrame);
    XC_PTR_ASSERT(key);
    return ((const core::OpaKey*)key)->data();
}

#if 0
const core::PoseKey::Data& getPoseKeyData(const core::ObjectNode& aTarget, int aFrame)
{
    auto key = aTarget.timeLine()->timeKey(core::TimeKeyType_Pose, aFrame);
    XC_PTR_ASSERT(key);
    return ((const core::PoseKey*)key)->data();
}

const core::FFDKey::Data& getFFDKeyData(const core::ObjectNode& aTarget, int aFrame)
{
    auto key = aTarget.timeLine()->timeKey(core::TimeKeyType_FFD, aFrame);
    XC_PTR_ASSERT(key);
    return ((const core::FFDKey*)key)->data();
}
#endif

} // namespace

namespace gui {
namespace prop {

KeyAccessor::KeyAccessor()
    : mProject()
    , mTarget()
{
}

void KeyAccessor::setProject(core::Project* aProject)
{
    mProject = aProject;
}

void KeyAccessor::setTarget(core::ObjectNode* aTarget)
{
    mTarget = aTarget;
}

//-------------------------------------------------------------------------------------------------
#define ASSERT_AND_RETURN_INVALID_TARGET() \
    XC_ASSERT(isValid());                  \
    if (!isValid()) return;                \

void KeyAccessor::assignSRTEasing(util::Easing::Param aNext)
{
    ASSERT_AND_RETURN_INVALID_TARGET();
    XC_ASSERT(aNext.isValidParam());
    const int frame = getFrame();
    auto newData = getSRTKeyData(*mTarget, frame);
    newData.easing = aNext;

    ctrl::TimeLineUtil::assignSRTKeyData(*mProject, *mTarget, frame, newData);
}

void KeyAccessor::assignSpline(int aNext)
{
    ASSERT_AND_RETURN_INVALID_TARGET();
    XC_ASSERT(0 <= aNext && aNext < core::SRTKey::SplineType_TERM);
    const int frame = getFrame();
    auto newData = getSRTKeyData(*mTarget, frame);
    newData.spline = (core::SRTKey::SplineType)aNext;

    ctrl::TimeLineUtil::assignSRTKeyData(*mProject, *mTarget, frame, newData);
}

void KeyAccessor::assignTrans(const QVector2D& aNewPos)
{
    ASSERT_AND_RETURN_INVALID_TARGET();
    const int frame = getFrame();
    auto newData = getSRTKeyData(*mTarget, frame);
    newData.pos.setX(aNewPos.x());
    newData.pos.setY(aNewPos.y());

    ctrl::TimeLineUtil::assignSRTKeyData(*mProject, *mTarget, frame, newData);
}

void KeyAccessor::assignRotate(float aRotate)
{
    ASSERT_AND_RETURN_INVALID_TARGET();
    const int frame = getFrame();
    auto newData = getSRTKeyData(*mTarget, frame);
    newData.rotate = aRotate;

    ctrl::TimeLineUtil::assignSRTKeyData(*mProject, *mTarget, frame, newData);
}

void KeyAccessor::assignScale(const QVector2D& aNewScale)
{
    ASSERT_AND_RETURN_INVALID_TARGET();
    const int frame = getFrame();
    auto newData = getSRTKeyData(*mTarget, frame);
    newData.scale = aNewScale;

    ctrl::TimeLineUtil::assignSRTKeyData(*mProject, *mTarget, frame, newData);
}

void KeyAccessor::assignOpacity(float aOpacity)
{
    ASSERT_AND_RETURN_INVALID_TARGET();
    const int frame = getFrame();
    auto newData = getOpaKeyData(*mTarget, frame);
    newData.opacity = aOpacity;

    ctrl::TimeLineUtil::assignOpaKeyData(*mProject, *mTarget, frame, newData);
}

void KeyAccessor::assignOpaEasing(util::Easing::Param aNext)
{
    ASSERT_AND_RETURN_INVALID_TARGET();
    XC_ASSERT(aNext.isValidParam());
    const int frame = getFrame();
    auto newData = getOpaKeyData(*mTarget, frame);
    newData.easing = aNext;

    ctrl::TimeLineUtil::assignOpaKeyData(*mProject, *mTarget, frame, newData);
}

void KeyAccessor::assignPoseEasing(util::Easing::Param aNext)
{
    ASSERT_AND_RETURN_INVALID_TARGET();
    XC_ASSERT(aNext.isValidParam());
    ctrl::TimeLineUtil::assignPoseKeyEasing(*mProject, *mTarget, getFrame(), aNext);
}

void KeyAccessor::assignFFDEasing(util::Easing::Param aNext)
{
    ASSERT_AND_RETURN_INVALID_TARGET();
    XC_ASSERT(aNext.isValidParam());
    ctrl::TimeLineUtil::assignFFDKeyEasing(*mProject, *mTarget, getFrame(), aNext);
}

//-------------------------------------------------------------------------------------------------
void KeyAccessor::knockNewSRT()
{
    ASSERT_AND_RETURN_INVALID_TARGET();
    auto newKey = new core::SRTKey();
    newKey->data() = mTarget->timeLine()->current().srt().data();

    ctrl::TimeLineUtil::pushNewSRTKey(*mProject, *mTarget, getFrame(), newKey);
}

void KeyAccessor::knockNewOpacity()
{
    ASSERT_AND_RETURN_INVALID_TARGET();
    auto newKey = new core::OpaKey();
    newKey->data() = mTarget->timeLine()->current().opa();

    ctrl::TimeLineUtil::pushNewOpaKey(*mProject, *mTarget, getFrame(), newKey);
}

void KeyAccessor::knockNewPose()
{
    ASSERT_AND_RETURN_INVALID_TARGET();
    auto newKey = new core::PoseKey();
    newKey->data() = mTarget->timeLine()->current().pose();
    core::BoneKey* parentKey = mTarget->timeLine()->current().areaBoneKey();
    XC_PTR_ASSERT(parentKey);
    newKey->data().createBonesBy(*parentKey);

    ctrl::TimeLineUtil::pushNewPoseKey(*mProject, *mTarget, getFrame(), newKey, parentKey);
}

void KeyAccessor::knockNewFFD()
{
    ASSERT_AND_RETURN_INVALID_TARGET();
    auto newKey = new core::FFDKey();
    newKey->data() = mTarget->timeLine()->current().ffd();
    core::TimeKey* parentKey = mTarget->timeLine()->current().ffdMeshParent();

    ctrl::TimeLineUtil::pushNewFFDKey(*mProject, *mTarget, getFrame(), newKey, parentKey);
}

void KeyAccessor::knockNewImage(const img::ResourceHandle& aHandle)
{
    ASSERT_AND_RETURN_INVALID_TARGET();
    auto newKey = new core::ImageKey();
    newKey->setImage(aHandle);

    ctrl::TimeLineUtil::pushNewImageKey(*mProject, *mTarget, getFrame(), newKey);
}

} // namespace prop
} // namespace gui
