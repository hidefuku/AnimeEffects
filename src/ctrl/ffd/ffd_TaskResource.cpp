#include <QFile>
#include "gl/Global.h"
#include "gl/ExtendShader.h"
#include "ctrl/ffd/ffd_TaskResource.h"

namespace ctrl {
namespace ffd {

//-------------------------------------------------------------------------------------------------
TaskResource::TaskResource()
{
    XC_ASSERT(kType * kHardness == kVariation);
}

void TaskResource::setup(
        const QString& aBrushPath, const QString& aEraserPath, const QString& aBlurPath)
{
    // load brush shader
    {
        QString code;
        loadFile(aBrushPath, code);

        for (int hard = 0; hard < kHardness; ++hard)
        {
            buildShader(mProgram[kTypeDeformer * kHardness + hard], code, kTypeDeformer, hard);
        }
    }

    // load eraser shader
    {
        QString code;
        loadFile(aEraserPath, code);

        for (int hard = 0; hard < kHardness; ++hard)
        {
            buildShader(mProgram[kTypeEraser * kHardness + hard], code, kTypeEraser, hard);
        }
    }

    // load blur path
    {
        QString code;
        loadFile(aBlurPath, code);
        buildBlurShader(mBlurProgram, code);
    }
}

gl::EasyShaderProgram& TaskResource::program(int aType, int aHard)
{
    XC_ASSERT(aType < kType && aHard < kHardness);
    return mProgram[aType * kHardness + aHard];
}

const gl::EasyShaderProgram& TaskResource::program(int aType, int aHard) const
{
    XC_ASSERT(aType < kType && aHard < kHardness);
    return mProgram[aType * kHardness + aHard];
}

void TaskResource::loadFile(const QString& aPath, QString& aDstCode)
{
    QFile file(aPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        XC_FATAL_ERROR("FileIO Error", file.errorString(), aPath);
    }
    QTextStream in(&file);
    aDstCode = in.readAll();
}

void TaskResource::buildShader(
        gl::EasyShaderProgram& aProgram, const QString& aCode,
        int aType, int aHard)
{
    gl::Global::Functions& ggl = gl::Global::functions();

    gl::ExtendShader source;

    // parse shader source
    source.openFromText(aCode);

    // set variation
    source.setVariationValue("HARDNESS", QString::number(aHard));

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
    if (aType == kTypeDeformer)
    {
        static const GLchar* kVaryings[] = {
            "outPosition", "outWeight"
        };
        ggl.glTransformFeedbackVaryings(aProgram.id(), 2, kVaryings, GL_SEPARATE_ATTRIBS);
    }
    else
    {
        static const GLchar* kVaryings[] = {
            "outPosition"
        };
        ggl.glTransformFeedbackVaryings(aProgram.id(), 1, kVaryings, GL_SEPARATE_ATTRIBS);
    }

    // link shader
    if (!aProgram.link())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.", aProgram.log());
    }
    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
}

void TaskResource::buildBlurShader(
        gl::EasyShaderProgram& aProgram, const QString& aCode)
{
    gl::Global::Functions& ggl = gl::Global::functions();

    gl::ExtendShader source;

    // parse shader source
    source.openFromText(aCode);

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
    static const GLchar* kVaryings[] = {
        "outPosition"
    };
    ggl.glTransformFeedbackVaryings(aProgram.id(), 1, kVaryings, GL_SEPARATE_ATTRIBS);

    // link shader
    if (!aProgram.link())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.",
                       aProgram.log());
    }
    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
}

} // namespace ffd
} // namespace ctrl
