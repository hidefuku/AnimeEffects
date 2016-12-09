#include "cmnd/ScopedMacro.h"
#include "cmnd/BasicCommands.h"
#include "gl/Global.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/ffd/ffd_DrawMode.h"

using namespace core;

namespace ctrl {
namespace ffd {

//-------------------------------------------------------------------------------------------------
DrawMode::Status::Status()
    : state(State_Idle)
    , brush(QVector2D(), 1.0f)
    , commandRef()
{
}

void DrawMode::Status::clear()
{
    state = State_Idle;
    commandRef = nullptr;
}

bool DrawMode::Status::hasValidBrush() const
{
    return brush.radius() > 0.0f;
}

//-------------------------------------------------------------------------------------------------
DrawMode::DrawMode(core::Project& aProject, Targets& aTargets)
    : mProject(aProject)
    , mTargets(aTargets)
    , mParam()
    , mStatus()
{
}

void DrawMode::updateParam(const FFDParam& aParam)
{
    mStatus.brush.setRadius(
                aParam.type == 0 ? aParam.radius : aParam.eraseRadius);
    mParam = aParam;
}

bool DrawMode::updateCursor(const core::CameraInfo& aCamera, const core::AbstractCursor& aCursor)
{
    if (mStatus.state == State_Idle)
    {
        mStatus.brush.setCenter(aCursor.worldPos());

        if (aCursor.emitsLeftPressedEvent())
        {
            mStatus.commandRef = nullptr;
            if (mStatus.hasValidBrush() && mTargets.hasValidTarget())
            {
                mStatus.state = State_Draw;
            }
        }
    }
    else if (mStatus.state == State_Draw)
    {
        const QVector2D prevCenter = mStatus.brush.center();
        mStatus.brush.setCenter(aCursor.worldPos());

        // deform
        if (aCursor.emitsLeftDraggedEvent())
        {
            // execute task
            if (!executeDrawTask(prevCenter, aCursor.worldVel()))
            {
                mStatus.clear();
            }
        }
        else if (aCursor.emitsLeftReleasedEvent())
        {
            mStatus.clear();
        }
    }

    return true;
}

void DrawMode::renderQt(const core::RenderInfo& aInfo, QPainter& aPainter)
{
    // draw brush
    {
        const QColor idleColor(100, 100, 255, 128);
        const QColor focusColor(255, 255, 255, 255);

        const QBrush centerBrush(mStatus.isDrawing() ? focusColor : idleColor);
        Qt::PenStyle style = mParam.type == 0 ? Qt::SolidLine : Qt::DotLine;
        aPainter.setPen(QPen(centerBrush, 1.2f, style));
        aPainter.setBrush(Qt::NoBrush);

        const QVector2D center = aInfo.camera.toScreenPos(mStatus.brush.center());
        const float radius = aInfo.camera.toScreenLength(mStatus.brush.radius());
        aPainter.drawEllipse(center.toPointF(), radius, radius);
    }
}

bool DrawMode::executeDrawTask(const QVector2D& aCenter, const QVector2D& aMove)
{
    static const size_t kCopySize = 1024;

    // setup input buffers
    gl::Global::makeCurrent();

    // request gl task
    for (int i = 0; i < mTargets.size(); ++i)
    {
        ObjectNode* node = mTargets[i]->node;
        LayerMesh* mesh = mTargets[i]->keyOwner.getParentMesh(node);
        ffd::KeyOwner& owner = mTargets[i]->keyOwner;
        FFDKey* key = owner.key;
        XC_PTR_ASSERT(node);
        XC_PTR_ASSERT(mesh);
        XC_PTR_ASSERT(key);
        XC_ASSERT(mesh->vertexCount() == key->data().count());

        // write vertex positions
        mTargets[i]->task->writeSrc(
                    node->timeLine()->current(),
                    key->data().positions(),
                    *mesh, mParam);

        // set brush
        mTargets[i]->task->setBrush(aCenter, aMove);

        // execute
        mTargets[i]->task->request();
    }

    // make commands
    {
        cmnd::Stack& stack = mProject.commandStack();
        const int frame = mProject.animator().currentFrame().get();

        if (!mStatus.commandRef)
        {
            cmnd::ScopedMacro macro(stack, "update ffd");

            // set notifier
            auto notifier = new TimeLineUtil::Notifier(mProject);
            macro.grabListener(notifier);
            notifier->event().setType(TimeLineEvent::Type_PushKey);
            for (int i = 0; i < mTargets.size(); ++i)
            {
                XC_PTR_ASSERT(mTargets[i]->node->timeLine());
                notifier->event().pushTarget(
                            *mTargets[i]->node, TimeKeyType_FFD, frame);
            }

            // push owns keys
            for (int i = 0; i < mTargets.size(); ++i)
            {
                XC_PTR_ASSERT(mTargets[i]->keyOwner.key);

                if (mTargets[i]->keyOwner.owns())
                {
                    mTargets[i]->keyOwner.pushOwnsKey(
                                stack, *mTargets[i]->node->timeLine(), frame);
                }
            }

            // create deform command
            mStatus.commandRef = new ffd::MoveVertices();

            // push memory assign commands
            for (int i = 0; i < mTargets.size(); ++i)
            {
                FFDKey* key = mTargets[i]->keyOwner.key;
                ffd::Task* task = mTargets[i]->task.data();
                XC_PTR_ASSERT(key->data().positions());
                XC_PTR_ASSERT(task->dstMesh());

                // wait end of task
                task->finish();

                mStatus.commandRef->push(
                            new cmnd::AssignMemory(
                                key->data().positions(),
                                task->dstMesh(), task->dstSize(), kCopySize));
            }

            // push deform command
            stack.push(mStatus.commandRef);
        }
        else
        {
            const bool modifiable = stack.isModifiable(mStatus.commandRef);
            TimeLineEvent event;
            event.setType(TimeLineEvent::Type_ChangeKeyValue);

            for (int i = 0; i < mTargets.size(); ++i)
            {
                FFDKey* key = mTargets[i]->keyOwner.key; (void)key;
                ffd::Task* task = mTargets[i]->task.data();
                ObjectNode& node = *mTargets[i]->node;

                // wait end of task
                task->finish();

                if (modifiable)
                {
                    // modify value
                    auto assign = mStatus.commandRef->assign(i);
                    XC_ASSERT(assign->size() == task->dstSize());
                    XC_ASSERT(assign->target() == key->data().positions());
                    assign->modifyValue(task->dstMesh());

                    // push event target
                    event.pushTarget(node, TimeKeyType_FFD, frame);
                }
            }

            // notify
            if (event.targets().size())
            {
                mProject.onTimeLineModified(event, false);
            }

            return modifiable;
        }
    }
    return true;
}

} // namespace ffd
} // namespace ctrl
