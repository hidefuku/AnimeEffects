#ifndef CTRL_POSE_TRANSBONEMODE_H
#define CTRL_POSE_TRANSBONEMODE_H

#include <QMatrix4x4>
#include "core/Project.h"
#include "core/ObjectNode.h"
#include "core/BoneKey.h"
#include "core/AbstractCursor.h"
#include "ctrl/bone/bone_Focuser.h"
#include "ctrl/pose/pose_KeyOwner.h"
#include "ctrl/pose/pose_RotateBone.h"
#include "ctrl/pose/pose_Target.h"

namespace ctrl {
namespace pose {

class TransBoneMode
{
public:
    TransBoneMode(core::Project& aProject, const Target& aTarget, KeyOwner& aKey);
    bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&);
    void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    void rotateBone(core::Bone2& aTarget, float aRotate);

    core::Project& mProject;
    core::ObjectNode& mTarget;
    QMatrix4x4 mTargetMtx;
    QMatrix4x4 mTargetInvMtx;
    KeyOwner& mKeyOwner;
    bone::Focuser mFocuser;
    RotateBone* mCommandRef;
    QVector2D mMoveOffset;
};

} // namespace pose
} // namespace ctrl

#endif // CTRL_POSE_TRANSBONEMODE_H
