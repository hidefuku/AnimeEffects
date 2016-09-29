#include <QFile>
#include "gl/Global.h"
#include "gl/ExtendShader.h"
#include "core/MeshTransformerResource.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
MeshTransformerResource::MeshTransformerResource()
{
}

void MeshTransformerResource::setup(const QString& aShaderPath)
{
    QString code;
    loadFile(aShaderPath, code);

    buildShader(mProgram[0], code, false, false);
    buildShader(mProgram[1], code, true, false);
    buildShader(mProgram[2], code, true, true);
}

gl::EasyShaderProgram& MeshTransformerResource::program(bool aUseSkinning, bool aUseDualQuaternion)
{
    return !aUseSkinning ? mProgram[0] :
        (!aUseDualQuaternion ? mProgram[1] : mProgram[2]);
}

const gl::EasyShaderProgram& MeshTransformerResource::program(bool aUseSkinning, bool aUseDualQuaternion) const
{
    return !aUseSkinning ? mProgram[0] :
        (!aUseDualQuaternion ? mProgram[1] : mProgram[2]);
}


void MeshTransformerResource::loadFile(const QString& aPath, QString& aDstCode)
{
    QFile file(aPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        XC_FATAL_ERROR("FileIO Error", file.errorString(), aPath);
    }
    QTextStream in(&file);
    aDstCode = in.readAll();
}

void MeshTransformerResource::buildShader(
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

} // namespace core
