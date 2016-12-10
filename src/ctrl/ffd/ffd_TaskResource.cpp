#include <QFile>
#include "gl/Global.h"
#include "gl/ExtendShader.h"
#include "ctrl/ffd/ffd_TaskResource.h"

namespace ctrl {
namespace ffd {

//-------------------------------------------------------------------------------------------------
TaskResource::TaskResource()
{
}

void TaskResource::setup(
        const QString& aBrushPath, const QString& aEraserPath,
        const QString& aFocuserPath, const QString& aBlurPath)
{
    // load brush shader
    {
        QString code;
        loadFile(aBrushPath, code);

        for (int hard = 0; hard < kHardness; ++hard)
        {
            buildShader(mProgram[kTypeDeformer * kHardness + hard],
                    code, kTypeDeformer, hard);
        }
    }

    // load eraser shader
    {
        QString code;
        loadFile(aEraserPath, code);

        for (int hard = 0; hard < kHardness; ++hard)
        {
            buildShader(mProgram[kTypeEraser * kHardness + hard],
                    code, kTypeEraser, hard);
        }
    }

    // load focuser shader
    {
        QString code;
        loadFile(aFocuserPath, code);

        buildShader(mProgram[kTypeFocuser * kHardness],
                code, kTypeFocuser, 0);
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
    switch (aType)
    {
    case kTypeDeformer: return mProgram[kTypeDeformer * kHardness + aHard];
    case kTypeEraser: return mProgram[kTypeEraser * kHardness + aHard];
    case kTypeFocuser: return mProgram[kTypeFocuser * kHardness + 0];
    default: XC_ASSERT(0); return mProgram[0];
    }
}

const gl::EasyShaderProgram& TaskResource::program(int aType, int aHard) const
{
    switch (aType)
    {
    case kTypeDeformer: return mProgram[kTypeDeformer * kHardness + aHard];
    case kTypeEraser: return mProgram[kTypeEraser * kHardness + aHard];
    case kTypeFocuser: return mProgram[kTypeFocuser * kHardness + 0];
    default: XC_ASSERT(0); return mProgram[0];
    }
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
    if (aType != kTypeFocuser)
    {
        source.setVariationValue("HARDNESS", QString::number(aHard));
    }

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
    else if (aType == kTypeEraser)
    {
        static const GLchar* kVaryings[] = {
            "outPosition"
        };
        ggl.glTransformFeedbackVaryings(aProgram.id(), 1, kVaryings, GL_SEPARATE_ATTRIBS);
    }
    else if (aType == kTypeFocuser)
    {
        static const GLchar* kVaryings[] = {
            "outPosition", "outWeight"
        };
        ggl.glTransformFeedbackVaryings(aProgram.id(), 2, kVaryings, GL_SEPARATE_ATTRIBS);
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
