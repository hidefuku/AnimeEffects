#ifndef CTRL_BONE_BINDNODESMODE_H
#define CTRL_BONE_BINDNODESMODE_H

#include "core/Project.h"
#include "core/ObjectNode.h"
#include "core/BoneKey.h"
#include "ctrl/GraphicStyle.h"
#include "ctrl/bone/bone_IMode.h"
#include "ctrl/bone/bone_KeyOwner.h"
#include "ctrl/bone/bone_Target.h"
#include "ctrl/bone/bone_Focuser.h"

namespace ctrl {
namespace bone {

class BindNodesMode : public IMode
{
public:
    BindNodesMode(core::Project& aProject, const Target& aTarget,
                  KeyOwner& aKey, const GraphicStyle& aGraphicStyle);
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&);
    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    void renderChildNodes(const core::RenderInfo& aInfo, QPainter& aPainter);
    core::Project& mProject;
    const GraphicStyle& mGraphicStyle;
    core::ObjectNode& mTarget;
    KeyOwner& mKeyOwner;
    QMatrix4x4 mTargetMtx;
    QMatrix4x4 mTargetInvMtx;
    Focuser mFocuser;
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_BINDNODESMODE_H
