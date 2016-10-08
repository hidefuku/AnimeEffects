#include "cmnd/ScopedMacro.h"
#include "cmnd/BasicCommands.h"
#include "gl/Global.h"
#include "core/MeshTransformerResource.h"
#include "ctrl/FFDEditor.h"
#include "ctrl/TimeLineUtil.h"

#include "gl/Util.h"
#include "gl/ComputeTexture1D.h"

using namespace core;

namespace ctrl
{

//-------------------------------------------------------------------------------------------------
FFDEditor::Status::Status()
    : state(State_Idle)
    , brush(QVector2D(), 1.0f)
    , commandRef()
{
}

void FFDEditor::Status::clear()
{
    state = State_Idle;
    commandRef = nullptr;
}

bool FFDEditor::Status::hasValidBrush() const
{
    return brush.radius() > 0.0f;
}

//-------------------------------------------------------------------------------------------------
FFDEditor::Target::Target()
    : node()
    , keyOwner()
    , task()
{
}

FFDEditor::Target::Target(core::ObjectNode* aNode)
    : node(aNode)
    , keyOwner()
    , task()
{
}

bool FFDEditor::Target::isValid() const
{
    return (bool)node && (bool)keyOwner && keyOwner.hasInv;
}

FFDEditor::Target::~Target()
{
    keyOwner.deleteOwnsKey();

    gl::Global::makeCurrent();
    task.reset();
}

//-------------------------------------------------------------------------------------------------
FFDEditor::FFDEditor(Project& aProject, DriverResources& aDriverResources)
    : mProject(aProject)
    , mDriverResources(aDriverResources)
    , mParam()
    , mRootTarget()
    , mTargets()
    , mStatus()
{
    mStatus.brush.setRadius(mParam.radius);

    // setup shader
    if (!mDriverResources.meshTransformerResource())
    {
        gl::Global::makeCurrent();
        mDriverResources.grabMeshTransformerResoure(new core::MeshTransformerResource());
        mDriverResources.meshTransformerResource()->setup("./data/shader/MeshTransform.glslex");
    }
    if (!mDriverResources.ffdTaskResource())
    {
        gl::Global::makeCurrent();
        mDriverResources.grabFFDTaskResource(new ffd::TaskResource());
        mDriverResources.ffdTaskResource()->setup(
                    "./data/shader/FreeFormDeform.glslex",
                    "./data/shader/FFDErase.glslex",
                    "./data/shader/FFDBlur.glslex");
    }

#if 0
    gl::Global::Functions& ggl = gl::Global::functions();
    gl::EasyShaderProgram program;
    {
        gl::ExtendShader source;

        // parse shader source
        if (!source.openFromFile("./data/shader/TestBlur.glslex"))
        {
            XC_FATAL_ERROR("OpenGL Error", "Failed to open shader.", source.log());
        }

        // resolve variation
        if (!source.resolveVariation())
        {
            XC_FATAL_ERROR("OpenGL Error", "Failed to resolve shader variation.",
                           source.log());
        }

        // set shader source
        program.setVertexSource(source);

        // feedback
        static const GLchar* kVaryings[] = {
            "outPosition"
        };
        ggl.glTransformFeedbackVaryings(
                    program.id(), 1, kVaryings, GL_SEPARATE_ATTRIBS);

        // link shader
        if (!program.link())
        {
            XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.", program.log());
        }
        XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
    }

    static const int inCount = 8;

    gl::BufferObject outPosition(GL_TRANSFORM_FEEDBACK_BUFFER);
    outPosition.resetData<gl::Vector3>(inCount, GL_STREAM_READ);

    gl::ComputeTexture1D texture(gl::ComputeTexture1D::CompoType_F32, 3);
    static const gl::Vector3 texPosition[] = {
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 2.0f, 0.0f },
        { 0.0f, 3.0f, 0.0f },
        { 0.0f, 4.0f, 0.0f },
        { 0.0f, 5.0f, 0.0f },
        { 0.0f, 6.0f, 0.0f },
        { 0.0f, 7.0f, 0.0f }
    };
    static const gl::Vector3 texPosition2[] = {
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 1.0f },
        { 2.0f, 0.0f, 2.0f },
        { 3.0f, 0.0f, 3.0f },
        { 4.0f, 0.0f, 4.0f },
        { 5.0f, 0.0f, 5.0f },
        { 6.0f, 0.0f, 6.0f },
        { 7.0f, 0.0f, 7.0f }
    };
    //texture.create(texPosition, inCount);
    texture.create(texPosition, inCount);
    texture.update(texPosition2);

    gl::Util::resetRenderState();
    ggl.glEnable(GL_RASTERIZER_DISCARD);
    //ggl.glEnable(GL_TEXTURE_1D);
    ggl.glActiveTexture(GL_TEXTURE0);
    ggl.glBindTexture(GL_TEXTURE_1D, texture.id());
    {
        static const gl::Vector3 inPosition[] = {
            { 0.0f, 0.0f, 7.0f },
            { 1.0f, 0.0f, 6.0f },
            { 2.0f, 0.0f, 5.0f },
            { 3.0f, 0.0f, 4.0f },
            { 4.0f, 0.0f, 3.0f },
            { 5.0f, 0.0f, 2.0f },
            { 6.0f, 0.0f, 1.0f },
            { 7.0f, 0.0f, 0.0f }
        };

        static const float inIndex[] = {
            0, 1, 2, 3, 4, 5, 6, 7
        };

        program.bind();

        program.setAttributeArray("inPosition", inPosition);
        program.setAttributeArray("inIndex", inIndex);
        program.setUniformValue("uTexture", 0);
        program.setUniformValue("uCount", inCount);

        ggl.glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, outPosition.id());

        ggl.glBeginTransformFeedback(GL_POINTS);
        ggl.glDrawArrays(GL_POINTS, 0, inCount);
        ggl.glEndTransformFeedback();

        program.release();
    }
    ggl.glDisable(GL_RASTERIZER_DISCARD);
    ggl.glActiveTexture(GL_TEXTURE0);
    ggl.glBindTexture(GL_TEXTURE_1D, 0);
    //ggl.glDisable(GL_TEXTURE_1D);

    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);

    QScopedArrayPointer<gl::Vector3> destPosition;
    destPosition.reset(new gl::Vector3[inCount]);

    outPosition.bind();
    ggl.glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(gl::Vector3) * inCount, destPosition.data());
    outPosition.release();

    for (int i = 0; i < inCount; ++i)
    {
        qDebug() << "result" << destPosition[i].pos();
    }

    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
#endif
}

FFDEditor::~FFDEditor()
{
    clearState();
    qDeleteAll(mTargets);
    mTargets.clear();
}

void FFDEditor::setTarget(ObjectNode* aTarget)
{
    clearState();
    qDeleteAll(mTargets);
    mTargets.clear();
    mRootTarget = aTarget;

    if (mRootTarget)
    {
        if (!resetCurrentTarget())
        {
            mRootTarget = nullptr;
        }
    }
}

void FFDEditor::updateParam(const FFDParam& aParam)
{
    if (aParam.type == 0)
    {
        mStatus.brush.setRadius(aParam.radius);
    }
    else
    {
        mStatus.brush.setRadius(aParam.eraseRadius);
    }

    mParam = aParam;
}

bool FFDEditor::updateCursor(const CameraInfo&, const AbstractCursor& aCursor)
{
    if (!mRootTarget) return false;

    if (mStatus.state == State_Idle)
    {
        mStatus.brush.setCenter(aCursor.worldPos());

        if (aCursor.isLeftPressState())
        {
            mStatus.commandRef = nullptr;
            if (mStatus.hasValidBrush() && hasValidTarget())
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
        if (aCursor.isLeftMoveState())
        {
            // execute task
            if (!executeDrawTask(prevCenter, aCursor.worldVel()))
            {
                clearState();
            }
        }
        else if (aCursor.isLeftReleaseState())
        {
            clearState();
        }
    }

    return true;
}

void FFDEditor::updateEvent(EventType)
{
    resetCurrentTarget();
}

core::LayerMesh* FFDEditor::getCurrentAreaMesh(core::ObjectNode& aNode) const
{
    if (aNode.timeLine() && aNode.timeLine()->current().areaMesh())
    {
        return &aNode.timeLine()->current().areaMesh()->data();
    }
    return aNode.gridMesh();
}

bool FFDEditor::resetCurrentTarget()
{
    clearState();
    QVector<Target*> prevTargets = mTargets;
    mTargets.clear();

    gl::Global::makeCurrent();

    // reset targets
    for (ObjectNode::Iterator itr(mRootTarget); itr.hasNext();)
    {
        ObjectNode* node = itr.next();
        XC_PTR_ASSERT(node);
        LayerMesh* areaMesh = getCurrentAreaMesh(*node);

        if (node->timeLine() && areaMesh && areaMesh->vertexCount() > 0)
        {
            bool found = false;
            for (auto prev : prevTargets)
            {
                if (prev->node == node)
                {
                    prevTargets.removeOne(prev);
                    mTargets.push_back(prev);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                // push target
                mTargets.push_back(new Target(node));
                // create task
                mTargets.back()->task.reset(
                            new ffd::Task(
                                *mDriverResources.ffdTaskResource(),
                                *mDriverResources.meshTransformerResource()));
            }
            mTargets.back()->task->resetDst(areaMesh->vertexCount());
        }
    }

    updateTargetsKeys();

    qDeleteAll(prevTargets);
    return !mTargets.isEmpty();
}

void FFDEditor::updateTargetsKeys()
{
    // initialize each target
    for (int i = 0; i < mTargets.size(); ++i)
    {
        ObjectNode* node = mTargets[i]->node;
        const TimeLine* line = node->timeLine();
        const LayerMesh* areaMesh = getCurrentAreaMesh(*node);
        MeshKey* areaKey = node->timeLine()->current().areaMesh();
        const int frame = mProject.animator().currentFrame().get();
        XC_PTR_ASSERT(line);
        XC_PTR_ASSERT(areaMesh);
        XC_ASSERT(areaMesh->vertexCount() > 0);
        XC_ASSERT(!areaKey || areaKey->data().vertexCount() > 0);

        // get key
        mTargets[i]->keyOwner.createKey(*line, *areaMesh, areaKey, frame);
        // setup srt
        mTargets[i]->keyOwner.setupMtx(*node, *line);
    }
}

bool FFDEditor::hasValidTarget() const
{
    for (int i = 0; i < mTargets.size(); ++i)
    {
        if (mTargets[i]->isValid()) return true;
    }
    return false;
}

void FFDEditor::clearState()
{
    mStatus.clear();
}

bool FFDEditor::executeDrawTask(const QVector2D& aCenter, const QVector2D& aMove)
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
        mTargets[i]->task->setBrush(
                    owner.worldMtx, owner.invMtx,
                    aCenter, aMove);

        // execute
        mTargets[i]->task->request();
    }

    // make commands
    {
        XC_PTR_ASSERT(mRootTarget);
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

void FFDEditor::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
{
    if (!mRootTarget) return;

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

} // namespace ctrl
