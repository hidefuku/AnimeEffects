#ifndef CTRL_POSE_BONEDYNAMICS_H
#define CTRL_POSE_BONEDYNAMICS_H

#include "ctrl/pose/pose_RigidBone.h"

namespace ctrl {
namespace pose {

class BoneDynamics
{
public:
    BoneDynamics(const core::Bone2& aTopBone);
    ~BoneDynamics();

    void setConduction(float aRate);

    RigidBone& rigidTopBone() { return *mRigidTopBone; }
    const RigidBone& rigidTopBone() const { return *mRigidTopBone; }
    QVector<float> rotationDifferences() const;

    void pullBone(RigidBone& aTarget, const QVector2D& aPull, float aPullPos);

private:
    QVector2D adjustByOriginConstraint(RigidBone& aTarget);
    void pullParentBones(RigidBone& aTarget, const QVector2D& aPull);
    void pullChildBonesRecursive(RigidBone& aTarget, const QVector2D& aPull);
    void adjustParentBones(RigidBone& aTarget);
    void adjustChildBonesRecursive(RigidBone& aTarget);
    void deleteAll();

    const core::Bone2& mTopBone;
    RigidBone* mRigidTopBone;
    float mConduction;
};

} // namespace pose
} // namespace ctrl

#endif // CTRL_POSE_BONEDYNAMICS_H
