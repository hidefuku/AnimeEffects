#include <QFile>
#include "gl/Global.h"
#include "gl/Util.h"
#include "gl/ExtendShader.h"
#include "core/MeshTransformer.h"
#include "ObjectNodeUtil.h"

namespace core
{
//-------------------------------------------------------------------------------------------------
MeshTransformer::Resource::Resource()
{
}

void MeshTransformer::Resource::setup(const QString& aShaderPath)
{
    QString code;
    loadFile(aShaderPath, code);

    buildShader(mProgram[0], code, false, false);
    buildShader(mProgram[1], code, true, false);
    buildShader(mProgram[2], code, true, true);
}

gl::EasyShaderProgram& MeshTransformer::Resource::program(bool aUseSkinning, bool aUseDualQuaternion)
{
    return !aUseSkinning ? mProgram[0] :
        (!aUseDualQuaternion ? mProgram[1] : mProgram[2]);
}

const gl::EasyShaderProgram& MeshTransformer::Resource::program(bool aUseSkinning, bool aUseDualQuaternion) const
{
    return !aUseSkinning ? mProgram[0] :
        (!aUseDualQuaternion ? mProgram[1] : mProgram[2]);
}


void MeshTransformer::Resource::loadFile(const QString& aPath, QString& aDstCode)
{
    QFile file(aPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        XC_FATAL_ERROR("FileIO Error", file.errorString(), aPath);
    }
    QTextStream in(&file);
    aDstCode = in.readAll();
}

void MeshTransformer::Resource::buildShader(
        gl::EasyShaderProgram& aProgram, const QString& aCode,
        bool aUseSkinning, bool aUseDualQuaternion)
{
    gl::Global::Functions& ggl = gl::Global::functions();

    gl::ExtendShader source;

    // parse shader source
    source.openFromText(aCode);

    // set variation
    source.setVariationValue("USE_SKINNING", QString::number(aUseSkinning ? 1 : 0));
    source.setVariationValue("USE_DUAL_QUATERNION", QString::number(aUseDualQuaternion ? 1 : 0));

    // resolve variation
    if (!source.resolveVariation())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to resolve shader variation.",
                       source.log());
    }

    // set shader source
    if (!aProgram.setVertexSource(source))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile vertex shader.",
                       aProgram.log());
    }

    // feedback
    {
        static const GLchar* kVaryings[] = {
            "outPosition", "outXArrow", "outYArrow"
        };
        ggl.glTransformFeedbackVaryings(aProgram.id(), 3, kVaryings, GL_SEPARATE_ATTRIBS);
    }

    // link shader
    if (!aProgram.link())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.",
                       aProgram.log());
    }

    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
}

//-------------------------------------------------------------------------------------------------
MeshTransformer::MeshTransformer(const QString& aShaderPath)
    : mResource()
    , mOutPositions()
    , mOutXArrows()
    , mOutYArrows()
{
    mResource.setup(aShaderPath);
}

void MeshTransformer::callGL(
        const TimeKeyExpans& aExpans, LayerMesh::MeshBuffer& aMeshBuffer,
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

    const BoneInfluenceMap* influence = aExpans.boneInfluence();
    const bool useInfluence = (bool)influence && aUseInfluence;
    const bool useDualQuaternion = true;
    BoneInfluenceMap::Accessor inflData;

    const int vtxCount = aPositions.count();

    if (useInfluence)
    {
        worldMatrix = aExpans.outerMatrix();
        innerMatrix = aExpans.innerMatrix();
        inflData = influence->accessor();

        XC_MSG_ASSERT(influence->vertexCount() == vtxCount,
                      "%d, %d", vtxCount, influence->vertexCount());
    }
    else
    {
        worldMatrix = aExpans.srt().worldMatrix();
        worldMatrix.translate(-ObjectNodeUtil::getCenterOffset3D(aExpans.srt()));
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
