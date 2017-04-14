#include "gl/ExtendShader.h"
#include "gl/Global.h"
#include "core/ShaderHolder.h"

namespace core
{

ShaderHolder::ShaderHolder()
    : mShaders()
    , mGridShaders()
    , mClipperShaders()
{
    mShaders.resize(img::BlendMode_TERM * 2);
    mGridShaders.resize(1);
    mClipperShaders.resize(2);
}

ShaderHolder::~ShaderHolder()
{
    for (auto p : mShaders)
    {
        if (p) delete p;
    }
    for (auto p : mGridShaders)
    {
        if (p) delete p;
    }
    for (auto p : mClipperShaders)
    {
        if (p) delete p;
    }
}

gl::EasyShaderProgram& ShaderHolder::reserveShader(img::BlendMode aBlendMode, bool aIsClippee)
{
    const int index = aBlendMode + img::BlendMode_TERM * (int)aIsClippee;
    if (!mShaders[index])
    {
        mShaders[index] = new gl::EasyShaderProgram();
        auto shader = mShaders[index];

        gl::ExtendShader source;
        if (!source.openFromFile("./data/shader/LayerDrawing.glslex"))
        {
            XC_FATAL_ERROR("FileIO Error", "Failed to open shader file.",
                           source.log());
        }

        auto blendFunc = QString("Blend") + img::getBlendFuncNameFromBlendMode(aBlendMode);
        source.setVariationValue("BLEND_FUNC", blendFunc);

        source.setVariationValue("IS_CLIPPEE", aIsClippee ? "1" : "0");

        if (!source.resolveVariation())
        {
            XC_FATAL_ERROR("OpenGL Error", "Failed to resolve shader variation.",
                           source.log());
        }

        if (!shader->setAllSource(source))
        {
            XC_FATAL_ERROR("OpenGL Error", "Failed to compile shader.",
                           shader->log());
        }

        if (!shader->link())
        {
            XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.",
                           shader->log());
        }
    }
    return *mShaders[index];
}

void ShaderHolder::reserveShaders(img::BlendMode aBlendMode)
{
    reserveShader(aBlendMode, true);
    reserveShader(aBlendMode, false);
}

gl::EasyShaderProgram& ShaderHolder::shader(img::BlendMode aBlendMode, bool aIsClippee)
{
    const int index = aBlendMode + img::BlendMode_TERM * (int)aIsClippee;
    XC_PTR_ASSERT(mShaders.at(index));
    return *(mShaders.at(index));
}

const gl::EasyShaderProgram& ShaderHolder::shader(img::BlendMode aBlendMode, bool aIsClippee) const
{
    const int index = aBlendMode + img::BlendMode_TERM * (int)aIsClippee;
    XC_PTR_ASSERT(mShaders.at(index));
    return *(mShaders.at(index));
}

gl::EasyShaderProgram& ShaderHolder::reserveGridShader()
{
    if (!mGridShaders[0])
    {
        mGridShaders[0] = new gl::EasyShaderProgram();
        auto shader = mGridShaders[0];

        gl::ExtendShader source;
        if (!source.openFromFile("./data/shader/GridDrawing.glslex"))
        {
            XC_FATAL_ERROR("FileIO Error", "Failed to open shader file.",
                           source.log());
        }
        if (!source.resolveVariation())
        {
            XC_FATAL_ERROR("OpenGL Error", "Failed to resolve shader variation.",
                           source.log());
        }

        if (!shader->setAllSource(source))
        {
            XC_FATAL_ERROR("OpenGL Error", "Failed to compile shader.",
                           shader->log());
        }

        if (!shader->link())
        {
            XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.",
                           shader->log());
        }
    }
    return *mGridShaders[0];
}

gl::EasyShaderProgram& ShaderHolder::gridShader()
{
    XC_PTR_ASSERT(mGridShaders.at(0));
    return *(mGridShaders.at(0));
}

const gl::EasyShaderProgram& ShaderHolder::gridShader() const
{
    XC_PTR_ASSERT(mGridShaders.at(0));
    return *(mGridShaders.at(0));
}

gl::EasyShaderProgram& ShaderHolder::reserveClipperShader(bool aIsClippee)
{
    if (!mClipperShaders.at(aIsClippee))
    {
        mClipperShaders[aIsClippee] = new gl::EasyShaderProgram();
        auto shader = mClipperShaders[aIsClippee];

        gl::ExtendShader source;
        if (!source.openFromFile("./data/shader/ClipperWriting.glslex"))
        {
            XC_FATAL_ERROR("FileIO Error", "Failed to open shader file.",
                           source.log());
        }

        source.setVariationValue("IS_CLIPPEE", aIsClippee ? "1" : "0");

        if (!source.resolveVariation())
        {
            XC_FATAL_ERROR("OpenGL Error", "Failed to resolve shader variation.",
                           source.log());
        }

        if (!shader->setAllSource(source))
        {
            XC_FATAL_ERROR("OpenGL Error", "Failed to compile shader.",
                           shader->log());
        }

        gl::Global::functions().glBindFragDataLocation(shader->id(), 0, "oClip");

        if (!shader->link())
        {
            XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.",
                           shader->log());
        }
    }
    return *mClipperShaders[aIsClippee];
}

void ShaderHolder::reserveClipperShaders()
{
    reserveClipperShader(true);
    reserveClipperShader(false);
}

gl::EasyShaderProgram& ShaderHolder::clipperShader(bool aIsClippee)
{
    XC_PTR_ASSERT(mClipperShaders.at(aIsClippee));
    return *(mClipperShaders.at(aIsClippee));
}

const gl::EasyShaderProgram& ShaderHolder::clipperShader(bool aIsClippee) const
{
    XC_PTR_ASSERT(mClipperShaders.at(aIsClippee));
    return *(mClipperShaders.at(aIsClippee));
}

} // namespace core

