#include <QElapsedTimer>
#include "XC.h"
#include "gl/Global.h"
#include "gl/Util.h"
#include "gl/Vector2I.h"
#include "gl/Vector3.h"
#include "core/Constant.h"
#include "ctrl/ffd/ffd_Task.h"

using namespace core;

namespace ctrl {
namespace ffd {

//-------------------------------------------------------------------------------------------------
Task::Task(TaskResource& aResource, core::MeshTransformerResource& aMeshRes)
    : mResource(aResource)
    , mMeshTransformer(aMeshRes)
    , mMeshBuffer()
    , mSrcExpans()
    , mArrayedConnectionList()
    , mSrcBlurPositions(gl::ComputeTexture1D::CompoType_F32, 2)
    , mWorkInMesh(GL_ARRAY_BUFFER)
    , mWorkInWeight(GL_ARRAY_BUFFER)
    , mOutMesh(GL_TRANSFORM_FEEDBACK_BUFFER)
    , mOutWeight(GL_TRANSFORM_FEEDBACK_BUFFER)
    , mOriginMesh()
    , mDstMesh()
    , mVtxCount()
    , mDstBufferCount()
    , mParam()
    , mBrushCenter()
    , mBrushVel()
    , mUseBlur()
{
    //GLint val;
    //glGetIntegerv(GL_MAX_TEXTURE_SIZE, &val);
    //qDebug() << val;
}

void Task::resetDst(int aVtxCount)
{
    XC_ASSERT(aVtxCount > 0);

    mVtxCount = aVtxCount;
    if (mDstBufferCount < aVtxCount)
    {
        mDstBufferCount = aVtxCount;
        mDstMesh.reset(new gl::Vector3[mVtxCount]);
        mOutMesh.resetData<gl::Vector3>(mVtxCount, GL_STREAM_COPY);
        mOutWeight.resetData<GLfloat>(mVtxCount, GL_STREAM_COPY);
    }
}

void Task::writeSrc(
        const TimeKeyExpans& aSrcExpans,
        const gl::Vector3* aSrcMesh,
        const LayerMesh& aOriginMesh,
        const FFDParam& aParam)
{
    XC_PTR_ASSERT(aSrcMesh);

    const int vtxCount = aOriginMesh.vertexCount();
    XC_ASSERT(vtxCount > 0);

    mSrcExpans = &aSrcExpans;
    mSrcMesh = util::ArrayBlock<const gl::Vector3>(aSrcMesh, vtxCount);

    mParam = aParam;
    mOriginMesh = aOriginMesh.positions();

    mUseBlur = aParam.type == TaskResource::kTypeDeformer && aParam.blur > 0.0f;
    if (mUseBlur)
    {
        mWorkInMesh.resetData<gl::Vector3>(vtxCount, GL_STREAM_COPY);
        mWorkInWeight.resetData<GLfloat>(vtxCount, GL_STREAM_COPY);

        // create connection data
        aOriginMesh.resetArrayedConnection(mArrayedConnectionList, aSrcMesh);

        // setup textures from connection data
        const int count = mArrayedConnectionList.blocks.count();
        mSrcBlurPositions.reset(count, LayerMesh::ArrayedConnection::kMaxCount);
        for (int i = 0; i < count; ++i)
        {
            auto block = mArrayedConnectionList.blocks.at(i);
            mSrcBlurPositions.at(i).update(
                        block->positions.data(), 0, block->positionCount);
        }
    }
}

void Task::setBrush(
        const QVector2D& aBrushCenter,
        const QVector2D& aBrushVel)
{
    mBrushCenter = aBrushCenter;
    mBrushVel = aBrushVel;
}

void Task::onRequested()
{
    XC_PTR_ASSERT(mSrcExpans);
    XC_ASSERT(mSrcMesh && mOutMesh && mParam.radius > 0.0f);
    XC_ASSERT(mSrcMesh.count() <= mOutMesh.dataCount());

    mMeshBuffer.reserve(mSrcMesh.count());
    mMeshTransformer.callGL(*mSrcExpans, mMeshBuffer, mSrcMesh);

    gl::Global::Functions& ggl = gl::Global::functions();
    gl::EasyShaderProgram& program = mResource.program(mParam.type, mParam.hardness);
    const int srcVtxCount = mSrcMesh.count();

    gl::Util::resetRenderState();
    ggl.glEnable(GL_RASTERIZER_DISCARD);
    {
        program.bind();

        if (mParam.type == TaskResource::kTypeDeformer)
        {
            program.setAttributeArray("inPosition", mSrcMesh.array(), srcVtxCount);
            program.setAttributeBuffer("inWorldPosition", mMeshTransformer.positions(), GL_FLOAT, 3);
            program.setAttributeBuffer("inXArrow", mMeshTransformer.xArrows(), GL_FLOAT, 3);
            program.setAttributeBuffer("inYArrow", mMeshTransformer.yArrows(), GL_FLOAT, 3);

            program.setUniformValue("uBrushCenter", mBrushCenter);
            program.setUniformValue("uBrushVel", mBrushVel);
            program.setUniformValue("uBrushRadius", (float)mParam.radius);
            program.setUniformValue("uBrushPressure", mParam.pressure);
            program.setUniformValue("uDividable", Constant::dividable());

            ggl.glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mOutMesh.id());
            ggl.glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, mOutWeight.id());
        }
        else
        {
            program.setAttributeArray("inPosition", mSrcMesh.array(), srcVtxCount);
            program.setAttributeBuffer("inWorldPosition", mMeshTransformer.positions(), GL_FLOAT, 3);
            program.setAttributeArray("inOriginPosition", mOriginMesh, srcVtxCount);

            program.setUniformValue("uBrushCenter", mBrushCenter);
            program.setUniformValue("uBrushRadius", (float)mParam.eraseRadius);
            program.setUniformValue("uBrushPressure", mParam.erasePressure);

            ggl.glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mOutMesh.id());
        }

        ggl.glBeginTransformFeedback(GL_POINTS);
        ggl.glDrawArrays(GL_POINTS, 0, mSrcMesh.count());
        ggl.glEndTransformFeedback();

        program.release();
    }
    ggl.glDisable(GL_RASTERIZER_DISCARD);

    if (mUseBlur)
    {
        requestBlur();
    }

    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
}

void Task::requestBlur()
{
    gl::Global::Functions& ggl = gl::Global::functions();
    gl::EasyShaderProgram& program = mResource.blurProgram();
    const int srcVtxCount = mSrcMesh.count();

    mWorkInMesh.copyFrom(mOutMesh);
    mWorkInWeight.copyFrom(mOutWeight);

    ggl.glEnable(GL_RASTERIZER_DISCARD);
    //ggl.glEnable(GL_TEXTURE_1D);
    ggl.glActiveTexture(GL_TEXTURE0);

    {
        int i = 0;
        for (auto block : mArrayedConnectionList.blocks)
        {
            const int begin = block->vertexRange.min();
            const int count = block->vertexRange.diff() + 1;

            if (count > 0)
            {
                program.bind();

                auto& texture = mSrcBlurPositions.at(i);
                ggl.glBindTexture(GL_TEXTURE_1D, texture.id());

                program.setAttributeBuffer("inPosition", mWorkInMesh, GL_FLOAT, 3);
                program.setAttributeBuffer("inWeight", mWorkInWeight, GL_FLOAT, 1);
                program.setAttributeArray("inOriginPosition", mOriginMesh, srcVtxCount);
                program.setAttributeArray("inIndexRange", mArrayedConnectionList.indexRanges.data(), srcVtxCount);

                program.setUniformValue("uConnections", 0);
                program.setUniformValue("uConnectionCount",
                                        LayerMesh::ArrayedConnection::kMaxCount);
                program.setUniformValue("uBlurPressure", (float)mParam.blur);

                //ggl.glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mOutMesh.id());
                ggl.glBindBufferRange(
                            GL_TRANSFORM_FEEDBACK_BUFFER, 0, mOutMesh.id(),
                            begin * sizeof(GLfloat) * 3, count * sizeof(GLfloat) * 3);

                ggl.glBeginTransformFeedback(GL_POINTS);
                ggl.glDrawArrays(GL_POINTS, begin, count);
                ggl.glEndTransformFeedback();

                program.release();
            }
            ++i;
        }
    }

    ggl.glActiveTexture(GL_TEXTURE0);
    ggl.glBindTexture(GL_TEXTURE_1D, 0);
    //ggl.glDisable(GL_TEXTURE_1D);
    ggl.glDisable(GL_RASTERIZER_DISCARD);

    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
}

void Task::onFinished()
{
    //QElapsedTimer timer; timer.start();

    gl::Global::Functions& ggl = gl::Global::functions();

    // read output mesh
    mOutMesh.bind();
    ggl.glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
                           sizeof(gl::Vector3) * mVtxCount, mDstMesh.data());
    mOutMesh.release();

    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
    //qDebug() << timer.nsecsElapsed();
}

} // namespace ffd
} // namespace ctrl
