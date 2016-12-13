#include "util/MathUtil.h"
#include "cmnd/BasicCommands.h"
#include "cmnd/ScopedMacro.h"
#include "core/Constant.h"
#include "core/TimeLine.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/CmndName.h"
#include "ctrl/mesh/mesh_CreateMode.h"
#include "ctrl/mesh/mesh_Renderer.h"
#include "ctrl/mesh/mesh_Notifier.h"

using namespace core;

namespace ctrl {
namespace mesh {

//-------------------------------------------------------------------------------------------------
CreateMode::CreateMode(Project& aProject, const Target& aTarget, KeyOwner& aKey)
    : mProject(aProject)
    , mTarget(*aTarget.node)
    , mKeyOwner(aKey)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mFocuser()
    , mMeshAccessor()
    , mState(State_Idle)
    , mDanglingPos()
    , mDanglingRef()
    , mCursorPos()
    , mLastFocus()
    , mMoverRef()
{
    XC_PTR_ASSERT(mKeyOwner.key);
    mMeshAccessor.setKey(*mKeyOwner.key);
    mFocuser.setMesh(mMeshAccessor);
    mFocuser.setTargetMatrix(mTargetMtx);

    initIdle();
}

bool CreateMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    bool updated = false;

    mFocuser.update(aCamera, aCursor);

    switch (mState)
    {
    case State_Idle: updated = procIdle(aCursor); break;
    case State_Move: procMove(aCursor); break;
    case State_New:  procNew(aCursor);  break;
    case State_Add:  procAdd(aCursor);  break;
    default: XC_ASSERT(0); break;
    }

    return updated || mState != State_Idle ||
            mFocuser.focusChanged() || aCursor.emitsRightPressedEvent();
}

void CreateMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
{
    Renderer renderer(aPainter, aInfo);
    renderer.setAntialiasing(true);
    renderer.setTargetMatrix(mTargetMtx);

    renderer.renderMesh(mMeshAccessor);

    if (!mDanglingPos.isEmpty())
    {
        renderer.renderDangling(mDanglingPos, mCursorPos);
    }

    renderer.renderFocus(mFocuser);
}

QVector2D CreateMode::getModelPos(const core::AbstractCursor& aCursor)
{
    return (mTargetInvMtx * QVector3D(aCursor.worldPos())).toVector2D();
}

MeshFace* CreateMode::pushTriangle(
        const QVector<QVector2D>& aPos, const QVector<MeshVtx*>& aRef)
{
    const int frame = mProject.animator().currentFrame().get();
    TimeLine& timeLine = *mTarget.timeLine();
    cmnd::Stack& stack = mProject.commandStack();
    auto eventType = mKeyOwner.owns() ?
                TimeLineEvent::Type_PushKey :
                TimeLineEvent::Type_ChangeKeyValue;

    MeshFace* faceCreated = nullptr;
    {
        cmnd::ScopedMacro macro(stack, CmndName::tr("push a triangle of a mesh key"));
        // set notifier
        macro.grabListener(new Notifier(mProject, mTarget, *mKeyOwner.key, eventType));

        // push key command
        if (mKeyOwner.owns())
        {
            mKeyOwner.pushOwnsKey(stack, timeLine, frame);

            // initialize gl mesh buffer
            mKeyOwner.key->data().getMeshBuffer();
        }

        // push command
        stack.push(mMeshAccessor.createTrianglePusher(aPos, aRef, &faceCreated));
    }
    ///@attention "faceCreated" was assigned when the command executed.

    return faceCreated;
}

void CreateMode::moveVtx(MeshVtx& aVtx, const QVector2D& aPos)
{
    cmnd::Stack& stack = mProject.commandStack();
    auto eventType = TimeLineEvent::Type_ChangeKeyValue;

    if (mMoverRef && mMoverRef->currentVtx() == &aVtx && stack.isModifiable(mMoverRef))
    {
        // modify
        mMoverRef->modify(aPos);

        // single shot
        Notifier notifier(mProject, mTarget, *mKeyOwner.key, eventType);
        notifier.notify();
    }
    else
    {
        cmnd::ScopedMacro macro(stack, CmndName::tr("move a vertex of a mesh key"));
        macro.grabListener(new Notifier(mProject, mTarget, *mKeyOwner.key, eventType));

        mMoverRef = new VtxMover(*mKeyOwner.key, aVtx, aPos);
        stack.push(mMoverRef);
    }
}

void CreateMode::initIdle()
{
    mState = State_Idle;
    mDanglingPos.clear();
    mDanglingRef.clear();
    mLastFocus = Focuser::Focus();
    mFocuser.setFocusEnable();
}

bool CreateMode::procIdle(const AbstractCursor& aCursor)
{
    auto pos = getModelPos(aCursor);
    mCursorPos = pos;

    if (aCursor.emitsRightPressedEvent())
    {
        mFocuser.clearFocus();
        mFocuser.clearSelection();
        return true;
    }

    if (aCursor.emitsLeftPressedEvent())
    {
        if (mFocuser.faceFocus())
        {
            mFocuser.selectFace(mFocuser.faceFocus());
            initAdd();
        }
        else if (mFocuser.vtxFocus())
        {
            initMove();
        }
        else
        {
            initNew(getModelPos(aCursor));
        }
        return true;
    }

    return false;
}

void CreateMode::initMove()
{
    mState = State_Move;
    mLastFocus = mFocuser.focus();
    mFocuser.setFocusEnable(false, false, false);
    mMoverRef = nullptr;
}

void CreateMode::procMove(const AbstractCursor& aCursor)
{

    auto pos = getModelPos(aCursor);
    mCursorPos = pos;

    if (aCursor.emitsRightPressedEvent())
    {
        mMoverRef = nullptr;
        mFocuser.clearFocus();
        mFocuser.clearSelection();
        initIdle();
        return;
    }

    if (mLastFocus.vtx)
    {
        moveVtx(*mLastFocus.vtx, getModelPos(aCursor));
    }

    if (aCursor.emitsLeftReleasedEvent())
    {
        mMoverRef = nullptr;
        initIdle();
    }
}

void CreateMode::initNew(const QVector2D& aModelPos)
{
    mState = State_New;

    if (mFocuser.vtxFocus())
    {
        mDanglingPos.push_back(mFocuser.vtxFocus()->vec());
        mDanglingRef.push_back(mFocuser.vtxFocus());
    }
    else
    {
        mDanglingPos.push_back(aModelPos);
        mDanglingRef.push_back(nullptr);
    }
    mCursorPos = aModelPos;
    mFocuser.setFocusEnable(true, false, false);
}

void CreateMode::procNew(const AbstractCursor& aCursor)
{
    auto pos = getModelPos(aCursor);
    mCursorPos = pos;
    if (mFocuser.vtxFocus())
    {
        mCursorPos = mFocuser.vtxFocus()->vec();
    }

    if (aCursor.emitsRightPressedEvent())
    {
        mFocuser.clearFocus();
        mFocuser.clearSelection();
        initIdle();
        return;
    }

    if (aCursor.emitsLeftPressedEvent())
    {
        if (mFocuser.vtxFocus())
        {
            mDanglingPos.push_back(mFocuser.vtxFocus()->vec());
            mDanglingRef.push_back(mFocuser.vtxFocus());
        }
        else
        {
            mDanglingPos.push_back(pos);
            mDanglingRef.push_back(nullptr);
        }
    }
    else if (aCursor.emitsLeftDraggedEvent())
    {
        if (!mDanglingRef.back())
        {
            mDanglingPos.back() = pos;
        }
    }
    else if (aCursor.emitsLeftReleasedEvent())
    {
        if (mDanglingPos.count() >= 3)
        {
            auto face = pushTriangle(mDanglingPos, mDanglingRef);
            mFocuser.selectFace(face);

            initAdd();
        }
    }
}

void CreateMode::initAdd()
{
    mState = State_Add;
    mDanglingPos.clear();
    mDanglingRef.clear();
    mFocuser.setFocusEnable(true, false, false);
}

void CreateMode::procAdd(const AbstractCursor& aCursor)
{
    auto pos = getModelPos(aCursor);
    mCursorPos = pos;
    if (mFocuser.vtxFocus())
    {
        mCursorPos = mFocuser.vtxFocus()->vec();
    }

    if (!mFocuser.selectingFace())
    {
        initIdle();
        return;
    }
    else if (aCursor.emitsRightPressedEvent())
    {
        mFocuser.clearFocus();
        mFocuser.clearSelection();
        initIdle();
        return;
    }

    // auto edge focus
    if (mFocuser.selectingFace() && mDanglingPos.count() < 3)
    {
        auto curPos = getModelPos(aCursor);
        auto face = mFocuser.selectingFace();
        MeshEdge* edge = face->nearestEdge(curPos);
        mFocuser.selectEdge(edge);

        mDanglingPos.clear();
        mDanglingRef.clear();
        if (edge)
        {
            // edge start
            mDanglingPos.push_back(edge->vtx(0)->vec());
            mDanglingRef.push_back(edge->vtx(0));

            // edge end
            mDanglingPos.push_back(edge->vtx(1)->vec());
            mDanglingRef.push_back(edge->vtx(1));
        }
    }

    if (aCursor.emitsLeftPressedEvent())
    {
        auto edge = mFocuser.selectingEdge();
        if (!edge)
        {
            initIdle();
            return;
        }

        if (mFocuser.vtxFocus())
        {
            mDanglingPos.push_back(mFocuser.vtxFocus()->vec());
            mDanglingRef.push_back(mFocuser.vtxFocus());
        }
        else
        {
            mDanglingPos.push_back(pos);
            mDanglingRef.push_back(nullptr);
        }
    }
    else if (aCursor.emitsLeftDraggedEvent())
    {
        if (!mDanglingRef.back())
        {
            mDanglingPos.back() = pos;
        }
    }
    else if (aCursor.emitsLeftReleasedEvent())
    {
        if (mDanglingPos.count() >= 3)
        {
            auto face = pushTriangle(mDanglingPos, mDanglingRef);
            mFocuser.selectFace(face);
            initAdd();
        }
    }
}

} // namespace mesh
} // namespace ctrl
