#include <cmath>
#include "util/TreeUtil.h"
#include "util/MathUtil.h"
#include "util/CollDetect.h"
#include "core/Constant.h"
#include "ctrl/pose/pose_BoneDynamics.h"

using namespace core;

namespace ctrl {
namespace pose {

//-------------------------------------------------------------------------------------------------
QVector2D pullBoneTail(RigidBone& aTarget, const QVector2D& aPull, float aConduction)
{
    QVector2D nextPull;
    if (aTarget.length >= core::Constant::normalizable())
    {
        auto normDir = aTarget.dir().normalized();
        auto trans = normDir * QVector2D::dotProduct(normDir, aPull);
        aTarget.force = aConduction * trans;
        nextPull = aTarget.force;

        auto rotate = aPull - trans;
        auto torque = (rotate.length() / aTarget.length) *
                (util::CollDetect::getCross(normDir, rotate) > 0.0f ? 1.0f : -1.0f);
        aTarget.torque = torque;
    }
    else
    {
        aTarget.force = aConduction * aPull;
        nextPull = aTarget.force;
    }
    aTarget.updateMotion(-1);
    return nextPull;
}

QVector2D pullBoneRoot(RigidBone& aTarget, const QVector2D& aPull, float aConduction)
{
    QVector2D nextPull;
    if (aTarget.length >= core::Constant::normalizable())
    {
        auto normDir = aTarget.dir().normalized();
        auto trans = normDir * QVector2D::dotProduct(normDir, aPull);
        aTarget.force = aConduction * trans;
        nextPull = aTarget.force;

        auto rotate = aPull - trans;
        auto torque = (rotate.length() / aTarget.length) *
                (util::CollDetect::getCross(normDir, rotate) < 0.0f ? 1.0f : -1.0f);
        aTarget.torque = torque;
    }
    else
    {
        aTarget.force = aConduction * aPull;
        nextPull = aTarget.force;
    }
    aTarget.updateMotion(1);
    return nextPull;
}

//-------------------------------------------------------------------------------------------------
BoneDynamics::BoneDynamics(const core::Bone2& aTopBone)
    : mTopBone(aTopBone)
    , mRigidTopBone()
    , mConduction(1.0f)
{
    mRigidTopBone = util::TreeUtil::createShadow<Bone2, RigidBone>(&mTopBone);
}

BoneDynamics::~BoneDynamics()
{
    deleteAll();
}

void BoneDynamics::setConduction(float aRate)
{
    mConduction = xc_clamp(aRate, 0.0f, 1.0f);
}

QVector<float> BoneDynamics::rotationDifferences() const
{
    using util::MathUtil;
    QVector<float> differences;

    RigidBone::ConstIterator itr(mRigidTopBone);
    while (itr.hasNext())
    {
        auto rigidBone = itr.next();
        float diff = 0.0f;
        if (rigidBone->parent())
        {
            auto rotate = rigidBone->angle - rigidBone->parent()->angle - rigidBone->ptr->localAngle();
            diff = MathUtil::getAngleDifferenceRad(
                        MathUtil::normalizeAngleRad(rigidBone->ptr->rotate()),
                        MathUtil::normalizeAngleRad(rotate));
        }
        differences.push_back(diff);
    }
    return differences;
}

void BoneDynamics::pullBone(RigidBone& aTarget, const QVector2D& aPull, float aPullPos)
{
    auto preRoot = aTarget.rootPos;
    auto preTail = aTarget.tailPos();
    if (aTarget.length >= core::Constant::normalizable())
    {
#if 1
        auto rotateRateLinear = std::abs(2.0f * (aPullPos - 0.5f));
        auto rotateRate = 1.0 - (1.0f - rotateRateLinear) * (1.0f - rotateRateLinear);
        rotateRate = mConduction * rotateRate + (1.0f - mConduction);

        auto normDir = aTarget.dir().normalized();
        auto vertical = normDir * QVector2D::dotProduct(normDir, aPull);
        auto horizontal = aPull - vertical;
        aTarget.force = mConduction * (vertical + (1.0f - rotateRate) * horizontal);

        auto rotate = horizontal * rotateRate;
        auto torque = (rotate.length() / aTarget.length) *
                (aPullPos >= 0.5f ? 1.0f : -1.0f) *
                (util::CollDetect::getCross(normDir, rotate) > 0.0f ? 1.0f : -1.0f);
        aTarget.torque = torque;
        aTarget.updateMotion(aPullPos >= 0.5f ? -1 : 1);
#else
        auto rotateRate = aPullPos;
        auto normDir = aTarget.dir().normalized();
        auto vertical = normDir * QVector2D::dotProduct(normDir, aPull);
        auto horizontal = aPull - vertical;
        aTarget.force = (vertical + (1.0f - rotateRate) * horizontal);

        auto rotate = horizontal * rotateRate;
        auto torque = (rotate.length() / aTarget.length) *
                (util::CollDetect::getCross(normDir, rotate) > 0.0f ? 1.0f : -1.0f);
        aTarget.torque = torque;
        aTarget.updateMotion(-1);
#endif
    }
    else
    {
        aTarget.force = aPull;
        aTarget.updateMotion();
    }
    // update parents
    pullParentBones(aTarget, aTarget.rootPos - preRoot);
    adjustByOriginConstraint(aTarget);

    // update children
    pullChildBonesRecursive(aTarget, aTarget.tailPos() - preTail);

    // adjustment
    for (int i = 0; i < 2; ++i)
    {
        adjustParentBones(aTarget);
        adjustChildBonesRecursive(aTarget);
    }
    // twig adjustment
    for (int i = 0; i < 2; ++i)
    {
        adjustChildBonesRecursive(*mRigidTopBone);
    }
}

QVector2D BoneDynamics::adjustByOriginConstraint(RigidBone& aTarget)
{
    QVector2D pull;
    if (aTarget.parent())
    {
        pull = adjustByOriginConstraint(*aTarget.parent());
    }
    else
    {
        auto originPos = mTopBone.worldPos();
        pull = originPos - aTarget.rootPos;
    }

    pull = pullBoneRoot(aTarget, pull, 1.0f);
    return pull;
}

void BoneDynamics::pullParentBones(RigidBone& aTarget, const QVector2D& aPull)
{
    auto pull = aPull;
    for (auto parent = aTarget.parent(); parent; parent = parent->parent())
    {
        pull = pullBoneTail(*parent, pull, mConduction);
    }
}

void BoneDynamics::pullChildBonesRecursive(RigidBone& aTarget, const QVector2D& aPull)
{
    for (auto child : aTarget.children())
    {
        auto nextPull = pullBoneRoot(*child, aPull, mConduction);
        pullChildBonesRecursive(*child, nextPull);
    }
}

void BoneDynamics::adjustParentBones(RigidBone& aTarget)
{
    auto prev = &aTarget;
    for (auto parent = aTarget.parent(); parent; parent = parent->parent())
    {
        auto pull = prev->rootPos - parent->tailPos();
        pullBoneTail(*parent, pull, 1.0f);
        prev = parent;
    }
}

void BoneDynamics::adjustChildBonesRecursive(RigidBone& aTarget)
{
    for (auto child : aTarget.children())
    {
        auto pull = aTarget.tailPos() - child->rootPos;
        pullBoneRoot(*child, pull, 1.0f);
        adjustChildBonesRecursive(*child);
    }
}

void BoneDynamics::deleteAll()
{
    if (mRigidTopBone)
    {
        util::TreeUtil::deleteAll(mRigidTopBone);
        mRigidTopBone = nullptr;
    }
}

} // namespace pose
} // namespace ctrl
