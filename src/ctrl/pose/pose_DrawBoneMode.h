#ifndef CTRL_POSE_DRAWBONEMODE_H
#define CTRL_POSE_DRAWBONEMODE_H

#include <QMatrix4x4>
#include "core/Project.h"
#include "core/ObjectNode.h"
#include "core/BoneKey.h"
#include "core/AbstractCursor.h"
#include "ctrl/bone/bone_Focuser.h"
#include "ctrl/pose/pose_IMode.h"
#include "ctrl/pose/pose_KeyOwner.h"
#include "ctrl/pose/pose_RotateBones.h"
#include "ctrl/pose/pose_Target.h"

#include "util/TreeNodeBase.h"

namespace ctrl {
namespace pose {

class RigidBone : public util::TreeNodeBase<RigidBone>
{
public:
    typedef util::TreeNodeBase<RigidBone>::Children ChildrenType;
    typedef util::TreeIterator<RigidBone, ChildrenType::Iterator> Iterator;
    typedef util::TreeIterator<const RigidBone, ChildrenType::ConstIterator> ConstIterator;

    RigidBone(const core::Bone2& aOrigin);

    QVector2D tailPos() const;
    QVector2D dir() const;
    void updateMotion(int aCentroid = 0);

    const core::Bone2* ptr;
    QVector2D rootPos;
    float angle;
    float length;
    QVector2D force;
    float torque;
};

class BoneDynamics
{
public:
    BoneDynamics(const core::Bone2& aTopBone);
    ~BoneDynamics();

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
    void updateMotions();
    void reconnectBones();
    void reconnectBonesRecursive(RigidBone& aCurrent, const QVector2D& aRootPull);

    const core::Bone2& mTopBone;
    RigidBone* mRigidTopBone;
    float mConduction;
};

class DrawBoneMode : public IMode
{
public:
    DrawBoneMode(core::Project& aProject, const Target& aTarget, KeyOwner& aKey);
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&);
    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    void pullBone(core::Bone2& aTarget, const QVector2D& aPull, float aPullPosRate);

    core::Project& mProject;
    core::ObjectNode& mTarget;
    QMatrix4x4 mTargetMtx;
    QMatrix4x4 mTargetInvMtx;
    KeyOwner& mKeyOwner;
    bone::Focuser mFocuser;
    RotateBones* mCommandRef;
    QVector2D mPullPos;
    QVector2D mPullOffset;
    float mPullPosRate;
};

} // namespace pose
} // namespace ctrl

#endif // CTRL_POSE_DRAWBONEMODE_H
