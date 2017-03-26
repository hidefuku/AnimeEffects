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

namespace ctrl {
namespace pose {

class DrawBoneMode : public IMode
{
public:
    DrawBoneMode(core::Project& aProject, const Target& aTarget, KeyOwner& aKey);
    virtual void updateParam(const PoseParam&);
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
    float mPullWeight;
};

} // namespace pose
} // namespace ctrl

#endif // CTRL_POSE_DRAWBONEMODE_H
