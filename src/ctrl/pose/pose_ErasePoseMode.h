#ifndef CTRL_POSE_ERASEPOSEMODE_H
#define CTRL_POSE_ERASEPOSEMODE_H

#include <QMatrix4x4>
#include "util/Circle.h"
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

class ErasePoseMode : public IMode
{
public:
    ErasePoseMode(core::Project& aProject, const Target& aTarget, KeyOwner& aKey);
    virtual void updateParam(const PoseParam&);
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&);
    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    void updatePaint();

    core::Project& mProject;
    core::ObjectNode& mTarget;
    QMatrix4x4 mTargetMtx;
    QMatrix4x4 mTargetInvMtx;
    KeyOwner& mKeyOwner;
    RotateAllBones* mCommandRef;
    util::Circle mBrush;
    float mBrushPressure;
    bool mIsBrushDrawing;
};

} // namespace pose
} // namespace ctrl

#endif // CTRL_POSE_ERASEPOSEMODE_H
