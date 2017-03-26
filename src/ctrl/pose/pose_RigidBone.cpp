#include "util/MathUtil.h"
#include "ctrl/pose/pose_RigidBone.h"

namespace ctrl {
namespace pose {

RigidBone::RigidBone(const core::Bone2& aOrigin)
    : TreeNodeBase(this)
    , ptr(&aOrigin)
    , rootPos()
    , angle()
    , length()
    , force()
    , torque()
{
    rootPos = aOrigin.parent() ? aOrigin.parent()->worldPos() : aOrigin.worldPos();
    length = (aOrigin.worldPos() - rootPos).length();
    angle = aOrigin.worldAngle();
}

QVector2D RigidBone::tailPos() const
{
    return rootPos + util::MathUtil::getVectorFromPolarCoord(length, angle);
}

QVector2D RigidBone::dir() const
{
    return util::MathUtil::getVectorFromPolarCoord(length, angle);
}

void RigidBone::updateMotion(int aCentroid)
{
    rootPos += force;
    if (aCentroid == -1)
    {
        angle += torque;
    }
    else if (aCentroid == 0)
    {
        auto rotate = torque;
        auto center = rootPos + 0.5f * dir();
        rootPos = center + util::MathUtil::getRotateVectorRad(rootPos - center, rotate);
        angle += rotate;
    }
    else if (aCentroid == 1)
    {
        auto rotate = torque;
        auto center = tailPos();
        rootPos = center + util::MathUtil::getRotateVectorRad(rootPos - center, rotate);
        angle += rotate;
    }

    force = QVector2D();
    torque = 0.0f;
}

} // namespace pose
} // namespace ctrl
