#include "gl/Global.h"
#include "core/MeshTransformerResource.h"
#include "ctrl/FFDEditor.h"
#include "ctrl/ffd/ffd_DrawMode.h"

#include "gl/Util.h"
#include "gl/ComputeTexture1D.h"

using namespace core;

namespace ctrl
{

//-------------------------------------------------------------------------------------------------
FFDEditor::FFDEditor(Project& aProject, DriverResources& aDriverResources)
    : mProject(aProject)
    , mDriverResources(aDriverResources)
    , mParam()
    , mCurrent()
    , mRootTarget()
    , mTargets()
{
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
    finalize();
}

void FFDEditor::finalize()
{
    mCurrent.reset();
    qDeleteAll(mTargets);
    mTargets.clear();
    mRootTarget = nullptr;
}

bool FFDEditor::setTarget(ObjectNode* aTarget)
{
    finalize();

    mRootTarget = aTarget;
    if (mRootTarget)
    {
        if (!resetCurrentTarget())
        {
            mRootTarget = nullptr;
        }
    }
    return mRootTarget;
}

void FFDEditor::updateParam(const FFDParam& aParam)
{
    if (mParam.type != aParam.type)
    {
        resetCurrentTarget();
    }
    if (mCurrent)
    {
        mCurrent->updateParam(aParam);
    }
    mParam = aParam;
}

bool FFDEditor::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    if (!mRootTarget) return false;

    if (mCurrent)
    {
        return mCurrent->updateCursor(aCamera, aCursor);
    }
    return false;

}

void FFDEditor::updateEvent(EventType)
{
    resetCurrentTarget();
}

core::LayerMesh* FFDEditor::getCurrentAreaMesh(core::ObjectNode& aNode) const
{
    if (!aNode.timeLine()) return nullptr;
    return aNode.timeLine()->current().ffdMesh();
}

bool FFDEditor::resetCurrentTarget()
{
    mCurrent.reset();
    QVector<ffd::Target*> prevTargets = mTargets;
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
                mTargets.push_back(new ffd::Target(node));
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

    if (mTargets.hasValidTarget())
    {
        createMode();
    }

    return !mTargets.isEmpty();
}

void FFDEditor::updateTargetsKeys()
{
    // initialize each target
    for (int i = 0; i < mTargets.size(); ++i)
    {
        ObjectNode* node = mTargets[i]->node;
        TimeLine* line = node->timeLine();
        XC_PTR_ASSERT(line);

        const int frame = mProject.animator().currentFrame().get();

        const LayerMesh* mesh = line->current().ffdMesh();
        XC_ASSERT(mesh && mesh->vertexCount() > 0);
        TimeKey* areaKey = line->current().ffdMeshParent();

        // get key
        mTargets[i]->keyOwner.createKey(*line, *mesh, areaKey, frame);
    }
}

void FFDEditor::createMode()
{
    mCurrent.reset();

    if (!mTargets.hasValidTarget()) return;

    switch (mParam.type)
    {
    case 0:
    case 1:
        mCurrent.reset(new ffd::DrawMode(mProject, mTargets));
        break;
    default:
        break;
    }

    if (mCurrent)
    {
        mCurrent->updateParam(mParam);
    }
}

void FFDEditor::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
{
    if (mCurrent)
    {
        mCurrent->renderQt(aInfo, aPainter);
    }
}

} // namespace ctrl
