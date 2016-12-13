#include "cmnd/ScopedMacro.h"
#include "cmnd/BasicCommands.h"
#include "gl/Global.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/CmndName.h"
#include "ctrl/ffd/ffd_DragMode.h"

namespace
{
static const float kFocusRadius = 5.0f;
}

using namespace core;

namespace ctrl {
namespace ffd {

//-------------------------------------------------------------------------------------------------
DragMode::DragMode(core::Project& aProject, Targets& aTargets)
    : mProject(aProject)
    , mTargets(aTargets)
    , mParam()
    , mState(State_Idle)
    , mFocusing()
    , mFocusTarget()
    , mFocusIndex()
    , mFocusPos()
    , mCommandRef()
{
}

void DragMode::updateParam(const FFDParam& aParam)
{
    mParam = aParam;
}

bool DragMode::updateCursor(const core::CameraInfo& aCamera, const core::AbstractCursor& aCursor)
{
    bool modified = false;
    auto prevFocus = mFocusing;
    auto prevFocusPos = mFocusPos;

    mParam.radius = aCamera.toWorldLength(kFocusRadius);

    if (mState == State_Idle)
    {
        mFocusing = executeFocusTask(aCursor.worldPos());

        if (aCursor.emitsLeftPressedEvent())
        {
            if (mFocusing)
            {
                mState = State_Drag;
            }
            else
            {
                mState = State_Miss;
            }
        }
    }
    else if (mState == State_Drag)
    {
        auto move = aCursor.worldVel();
        if (mFocusTarget && !move.isNull())
        {
            executeDragTask(move);
            modified = true;
        }

        if (aCursor.emitsLeftReleasedEvent())
        {
            mState = State_Idle;
            clearState();
        }
    }
    else if (mState == State_Miss)
    {
        if (aCursor.emitsLeftReleasedEvent())
        {
            mState = State_Idle;
            clearState();
        }
    }

    return modified || (mFocusing != prevFocus) || (mFocusPos != prevFocusPos);
}

void DragMode::clearState()
{
    mFocusing = false;
    mFocusTarget = nullptr;
    mFocusIndex = -1;
    mFocusPos = QVector2D();
    mCommandRef = nullptr;
}

bool DragMode::executeFocusTask(const QVector2D& aPos)
{
    // setup input buffers
    gl::Global::makeCurrent();

    // request gl task
    for (int i = 0; i < mTargets.size(); ++i)
    {
        auto task = mTargets[i]->task.data();
        ObjectNode* node = mTargets[i]->node;
        LayerMesh* mesh = mTargets[i]->keyOwner.getParentMesh(node);
        ffd::KeyOwner& owner = mTargets[i]->keyOwner;
        FFDKey* key = owner.key;
        XC_PTR_ASSERT(node);
        XC_PTR_ASSERT(mesh);
        XC_PTR_ASSERT(key);
        XC_ASSERT(mesh->vertexCount() == key->data().count());

        // set task type
        task->setType(Task::Type_Focuser);

        // write vertex positions
        task->writeSrc(
                    node->timeLine()->current(),
                    key->data().positions(),
                    *mesh, mParam);

        // set brush
        task->setBrush(aPos, QVector2D());

        // execute
        task->request();
    }

    bool found = false;
    for (int i = 0; i < mTargets.size(); ++i)
    {
        ffd::Task* task = mTargets[i]->task.data();

        // wait end of task
        task->finish();

        if (!found)
        {
            auto focusIndex = task->focusIndex();
            if (focusIndex >= 0)
            {
                mFocusTarget = mTargets[i];
                mFocusIndex = focusIndex;
                mFocusPos = (task->dstMesh())[focusIndex].pos2D();
                found = true;
            }
        }
    }
    return found;
}

void DragMode::executeDragTask(const QVector2D& aMove)
{
    XC_PTR_ASSERT(mFocusTarget);

    // setup input buffers
    gl::Global::makeCurrent();

    // request gl task
    {
        auto task = mFocusTarget->task.data();
        ObjectNode* node = mFocusTarget->node;
        LayerMesh* mesh = mFocusTarget->keyOwner.getParentMesh(node);
        ffd::KeyOwner& owner = mFocusTarget->keyOwner;
        FFDKey* key = owner.key;
        XC_PTR_ASSERT(node);
        XC_PTR_ASSERT(mesh);
        XC_PTR_ASSERT(key);
        XC_ASSERT(mesh->vertexCount() == key->data().count());

        // set task type
        task->setType(Task::Type_Dragger);
        task->setDragIndex(mFocusIndex);

        // write vertex positions
        task->writeSrc(
                    node->timeLine()->current(),
                    key->data().positions(),
                    *mesh, mParam);

        // set brush
        task->setBrush(mFocusPos, aMove);

        // execute
        task->request();

        // wait end of task
        task->finish();

        assignDragging(task->dragMove());

        mFocusPos += aMove;
    }
}

void DragMode::assignDragging(const QVector2D& aMove)
{
    XC_PTR_ASSERT(mFocusTarget);
    cmnd::Stack& stack = mProject.commandStack();
    const int frame = mProject.animator().currentFrame().get();

    auto ownsKey = mFocusTarget->keyOwner.owns();
    auto node = mFocusTarget->node;
    auto timeLine = node->timeLine();
    XC_PTR_ASSERT(timeLine);
    auto key = mFocusTarget->keyOwner.key;
    auto dstPosPtr = key->data().positions() + mFocusIndex;
    auto newPos = *dstPosPtr + gl::Vector3::make(aMove.x(), aMove.y(), 0.0f);

    if (!mCommandRef || !stack.isModifiable(mCommandRef))
    {
        cmnd::ScopedMacro macro(stack, CmndName::tr("move a vertex of a FFD key"));

        // set notifier
        auto notifier = new TimeLineUtil::Notifier(mProject);
        macro.grabListener(notifier);
        notifier->event().setType(
                    ownsKey ? TimeLineEvent::Type_PushKey :
                              TimeLineEvent::Type_ChangeKeyValue);
        notifier->event().pushTarget(*node, TimeKeyType_FFD, frame);

        // push owns key
        if (ownsKey)
        {
            mFocusTarget->keyOwner.pushOwnsKey(stack, *timeLine, frame);
        }

        // create command
        mCommandRef = new cmnd::ModifiableAssign<gl::Vector3>(dstPosPtr, newPos);

        stack.push(mCommandRef);
    }
    else
    {
        // modify value
        mCommandRef->modifyValue(newPos);

        // notify
        TimeLineEvent event;
        event.setType(TimeLineEvent::Type_ChangeKeyValue);
        event.pushTarget(*node, TimeKeyType_FFD, frame);
        mProject.onTimeLineModified(event, false);
    }
}

void DragMode::renderQt(const core::RenderInfo& aInfo, QPainter& aPainter)
{
    if (mFocusing)
    {
        aPainter.setPen(QPen(QColor(255, 255, 255, 255), 1.0f));
        aPainter.setBrush(QColor(64, 64, 255, 255));

        const QVector2D center = aInfo.camera.toScreenPos(mFocusPos);
        aPainter.drawEllipse(center.toPointF(), kFocusRadius, kFocusRadius);
    }
}

} // namespace ffd
} // namespace ctrl
