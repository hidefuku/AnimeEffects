#ifndef CTRL_BONE_MOVEJOINTMODE_H
#define CTRL_BONE_MOVEJOINTMODE_H

#include "core/Project.h"
#include "core/ObjectNode.h"
#include "core/BoneKey.h"
#include "ctrl/bone/bone_IMode.h"
#include "ctrl/bone/bone_KeyOwner.h"
#include "ctrl/bone/bone_Target.h"
#include "ctrl/bone/bone_Focuser.h"
#include "ctrl/bone/bone_MoveBone.h"

namespace ctrl {
namespace bone {

class MoveJointMode : public IMode
{
public:
    MoveJointMode(core::Project& aProject, const Target& aTarget, KeyOwner& aKey);
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&);
    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    void moveBone(core::Bone2& aTarget, const QVector2D& aMove);

    core::Project& mProject;
    core::ObjectNode& mTarget;
    KeyOwner& mKeyOwner;
    QMatrix4x4 mTargetMtx;
    QMatrix4x4 mTargetInvMtx;
    Focuser mFocuser;
    MoveBone* mCommandRef;
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_MOVEJOINTMODE_H
