#include "cmnd/ScopedMacro.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/bone/bone_BindNodesMode.h"
#include "ctrl/bone/bone_Renderer.h"
#include "ctrl/bone/bone_Notifier.h"

using namespace core;

namespace ctrl {
namespace bone {

BindNodesMode::BindNodesMode(Project& aProject, const Target& aTarget, KeyOwner& aKey)
    : mProject(aProject)
    , mTarget(*aTarget.node)
    , mKeyOwner(aKey)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mFocuser()
{
    XC_PTR_ASSERT(mKeyOwner.key);
    mFocuser.setTopBones(mKeyOwner.key->data().topBones());
    mFocuser.setTargetMatrix(mTargetMtx);
}

bool BindNodesMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    auto focus = mFocuser.update(aCamera, aCursor.screenPos());
    bool updated = mFocuser.focusChanged();

    if (aCursor.isLeftPressState())
    {
        mFocuser.clearFocus();
        mFocuser.clearSelection();
        if (focus)
        {
            mFocuser.select(*focus);
        }
        updated = true;
    }
    else if (aCursor.isLeftMoveState())
    {
    }
    else if (aCursor.isLeftReleaseState())
    {
        mFocuser.clearSelection();
        updated = true;
    }

    return updated;
}

void BindNodesMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
{
    bone::Renderer renderer(aPainter, aInfo);
    renderer.setAntialiasing(true);
    renderer.setTargetMatrix(mTargetMtx);

    for (auto bone : mKeyOwner.key->data().topBones())
    {
        renderer.renderBones(bone);
    }
}

} // namespace bone
} // namespace ctrl
