#include "gl/Global.h"
#include "gl/Util.h"
#include "core/MeshTransformer.h"
#include "core/MeshTransformerResource.h"
#include "core/ObjectNodeUtil.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
MeshTransformer::MeshTransformer(const QString& aShaderPath)
    : mResource(*(new MeshTransformerResource()))
    , mResourceOwns(true)
    , mOutPositions()
    , mOutXArrows()
    , mOutYArrows()
{
    mResource.setup(aShaderPath);
}

MeshTransformer::MeshTransformer(MeshTransformerResource& aResource)
    : mResource(aResource)
    , mResourceOwns(false)
    , mOutPositions()
    , mOutXArrows()
    , mOutYArrows()
{
}

MeshTransformer::~MeshTransformer()
{
    if (mResourceOwns)
    {
        delete &mResource;
    }
}

void MeshTransformer::callGL(
        const TimeKeyExpans& aExpans,
        LayerMesh::MeshBuffer& aMeshBuffer,
        const QVector2D& aOriginOffset,
        util::ArrayBlock<const gl::Vector3> aPositions,
        bool aNonPosed, bool aUseInfluence)
{
    XC_ASSERT(aPositions);

    auto& buffer = aMeshBuffer;
    const int outCount = buffer.vtxCount;
    mOutPositions = &buffer.outPositions;
    mOutXArrows = &buffer.outXArrows;
    mOutYArrows = &buffer.outYArrows;

    if (aPositions.count() <= 0) return;

    QMatrix4x4 worldMatrix;
    QMatrix4x4 innerMatrix;

    const BoneInfluenceMap* influence = aExpans.bone().influenceMap();
    bool useInfluence = aUseInfluence && influence && !aNonPosed;
    const bool useDualQuaternion = true;
    BoneInfluenceMap::Accessor inflData;

    const int vtxCount = aPositions.count();

    if (useInfluence)
    {
        inflData = influence->accessor();
        XC_MSG_ASSERT(influence->vertexCount() == vtxCount,
                      "%d, %d", vtxCount, influence->vertexCount());
    }

    if ((!aNonPosed && aExpans.bone().isAffectedByBinding()) || useInfluence)
    {
        worldMatrix = aExpans.bone().outerMatrix();
        innerMatrix = aExpans.bone().innerMatrix();
        innerMatrix.translate(aExpans.srt().centroid() + aOriginOffset);
    }
    else
    {
        worldMatrix = aExpans.srt().worldMatrix();
        worldMatrix.translate(aExpans.srt().centroid() + aOriginOffset);
    }

    gl::Global::Functions& ggl = gl::Global::functions();
    gl::EasyShaderProgram& program = mResource.program(useInfluence, useDualQuaternion);

    gl::Util::resetRenderState();
    ggl.glEnable(GL_RASTERIZER_DISCARD);
    {
        program.bind();

        program.setAttributeArray("inPosition", aPositions.array(), vtxCount);
        program.setUniformValue("uInnerMatrix", innerMatrix);
        program.setUniformValue("uWorldMatrix", worldMatrix);

        if (useInfluence)
        {
            program.setAttributeArray("inBoneIndex0", inflData.indices0(), vtxCount);
            program.setAttributeArray("inBoneWeight0", inflData.weights0(), vtxCount);
            program.setAttributeArray("inBoneIndex1", inflData.indices1(), vtxCount);
            program.setAttributeArray("inBoneWeight1", inflData.weights1(), vtxCount);

            if (useDualQuaternion)
            {
                auto palette = aNonPosed ? PosePalette().dualQuaternions() :
                                           aExpans.posePalette().dualQuaternions();
                program.setTupleUniformValueArray<GLfloat>(
                            "uBoneDualQuat",
                            palette.array()->data(),
                            palette.count() * 2, 4);
            }
            else
            {
                auto palette = aNonPosed ? PosePalette().matrices() :
                                           aExpans.posePalette().matrices();
                program.setUniformValueArray("uBoneMatrix", palette);
            }
        }

        ggl.glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffer.workPositions.id());
        ggl.glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, buffer.workXArrows.id());
        ggl.glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, buffer.workYArrows.id());

        ggl.glBeginTransformFeedback(GL_POINTS);
        ggl.glDrawArrays(GL_POINTS, 0, aPositions.count());
        ggl.glEndTransformFeedback();

        program.release();
    }
    ggl.glDisable(GL_RASTERIZER_DISCARD);
    GL_CHECK_ERROR();

    // copy to array buffer
    {
        ggl.glBindBuffer(GL_ARRAY_BUFFER, buffer.outPositions.id());
        ggl.glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, buffer.workPositions.id());
        ggl.glCopyBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, GL_ARRAY_BUFFER, 0, 0, sizeof(gl::Vector3) * outCount);

        ggl.glBindBuffer(GL_ARRAY_BUFFER, buffer.outXArrows.id());
        ggl.glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, buffer.workXArrows.id());
        ggl.glCopyBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, GL_ARRAY_BUFFER, 0, 0, sizeof(gl::Vector3) * outCount);

        ggl.glBindBuffer(GL_ARRAY_BUFFER, buffer.outYArrows.id());
        ggl.glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, buffer.workYArrows.id());
        ggl.glCopyBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, GL_ARRAY_BUFFER, 0, 0, sizeof(gl::Vector3) * outCount);

        ggl.glBindBuffer(GL_ARRAY_BUFFER, 0);
        ggl.glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
    }

    ggl.glFlush();

    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
}

} // namespace core
