#ifndef CTRL_BONE_CREATEMODE_H
#define CTRL_BONE_CREATEMODE_H

#include "core/Project.h"
#include "core/ObjectNode.h"
#include "core/BoneKey.h"
#include "ctrl/bone/bone_IMode.h"
#include "ctrl/bone/bone_KeyOwner.h"
#include "ctrl/bone/bone_Target.h"
#include "ctrl/bone/bone_Focuser.h"

namespace ctrl {
namespace bone {

class CreateMode : public IMode
{
public:
    CreateMode(core::Project& aProject, const Target& aTarget, KeyOwner& aKey);
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&);
    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    void pushNewBone(core::Bone2& aParent, core::Bone2& aNewChild);
    void pushNewTopBone(core::Bone2& aNewRoot, core::Bone2& aNewChild);

    core::Project& mProject;
    core::ObjectNode& mTarget;
    KeyOwner& mKeyOwner;
    QMatrix4x4 mTargetMtx;
    QMatrix4x4 mTargetInvMtx;
    QScopedPointer<core::Bone2> mDangledTop;
    Focuser mFocuser;
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_CREATEMODE_H
