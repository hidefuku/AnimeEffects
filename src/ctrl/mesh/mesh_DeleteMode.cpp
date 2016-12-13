#include "util/MathUtil.h"
#include "cmnd/BasicCommands.h"
#include "cmnd/ScopedMacro.h"
#include "core/Constant.h"
#include "core/TimeLine.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/CmndName.h"
#include "ctrl/mesh/mesh_DeleteMode.h"
#include "ctrl/mesh/mesh_Renderer.h"
#include "ctrl/mesh/mesh_Notifier.h"

using namespace core;

namespace ctrl {
namespace mesh {

//-------------------------------------------------------------------------------------------------
DeleteMode::DeleteMode(Project& aProject, const Target& aTarget, KeyOwner& aKey)
    : mProject(aProject)
    , mTarget(*aTarget.node)
    , mKeyOwner(aKey)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mFocuser()
    , mMeshAccessor()
{
    XC_PTR_ASSERT(mKeyOwner.key);
    mMeshAccessor.setKey(*mKeyOwner.key);
    mFocuser.setMesh(mMeshAccessor);
    mFocuser.setTargetMatrix(mTargetMtx);
    mFocuser.setFocusEnable(true, false, true);
}

bool DeleteMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    bool updated = false;
    mFocuser.update(aCamera, aCursor);

    if (aCursor.emitsLeftPressedEvent())
    {
        if (mFocuser.vtxFocus())
        {
            removeVtx(*mFocuser.vtxFocus());
        }
        else if (mFocuser.faceFocus())
        {
            removeFace(*mFocuser.faceFocus());
        }
        mFocuser.clearFocus();
        updated = true;
    }

    return updated || mFocuser.focusChanged();
}

void DeleteMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
{
    Renderer renderer(aPainter, aInfo);
    renderer.setAntialiasing(true);
    renderer.setTargetMatrix(mTargetMtx);

    renderer.renderMesh(mMeshAccessor);
    renderer.renderFocus(mFocuser);
}

QVector2D DeleteMode::getModelPos(const core::AbstractCursor& aCursor)
{
    return (mTargetInvMtx * QVector3D(aCursor.worldPos())).toVector2D();
}

void DeleteMode::removeVtx(MeshVtx& aVtx)
{
    cmnd::Stack& stack = mProject.commandStack();
    auto eventType = TimeLineEvent::Type_ChangeKeyValue;

    {
        cmnd::ScopedMacro macro(stack, CmndName::tr("remove a vertex of a mesh key"));
        // set notifier
        macro.grabListener(new Notifier(mProject, mTarget, *mKeyOwner.key, eventType));
        // push command
        stack.push(mMeshAccessor.createRemover(aVtx));
    }
}

void DeleteMode::removeFace(MeshFace& aFace)
{
    cmnd::Stack& stack = mProject.commandStack();
    auto eventType = TimeLineEvent::Type_ChangeKeyValue;

    {
        cmnd::ScopedMacro macro(stack, CmndName::tr("remove a face of a mesh key"));
        // set notifier
        macro.grabListener(new Notifier(mProject, mTarget, *mKeyOwner.key, eventType));
        // push command
        stack.push(mMeshAccessor.createRemover(aFace));
    }
}

} // namespace mesh
} // namespace ctrl
