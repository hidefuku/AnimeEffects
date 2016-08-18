#include "gl/Global.h"
#include "gl/ComputeTexture1D.h"

namespace gl
{
//-------------------------------------------------------------------------------------------------
ComputeTexture1D::ComputeTexture1D(CompoType aCompoType, int aCompoCount)
    : mCompoType(aCompoType)
    , mCompoCount(aCompoCount)
    , mId()
    , mSize()
    , mFormat()
    , mType()
{
    XC_ASSERT(1 <= aCompoCount && aCompoCount <= 4);
    XC_ASSERT(0 <= aCompoType && aCompoType < CompoType_TERM);
}

ComputeTexture1D::~ComputeTexture1D()
{
    destroy();
}

void ComputeTexture1D::create(const void* aData, GLsizei aSize)
{
    //XC_PTR_ASSERT(aData);
    XC_ASSERT(aSize > 0);
    destroy();

    mType = GLenum();
    mFormat = GLenum();
    GLint internalFormat = 0;

    switch (mCompoType)
    {
    case CompoType_S32:
    {
        const GLenum kFormats[] = {
            GL_RED_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER, GL_RGBA_INTEGER };
        const GLint kInternalFormats[] = { GL_R32I, GL_RG32I, GL_RGB32I, GL_RGBA32I };

        mType = GL_INT;
        mFormat = kFormats[mCompoCount - 1];
        internalFormat = kInternalFormats[mCompoCount - 1];
    } break;
    case CompoType_F32:
    {
        const GLenum kFormats[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
        const GLint kInternalFormats[] = { GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F };

        mType = GL_FLOAT;
        mFormat = kFormats[mCompoCount - 1];
        internalFormat = kInternalFormats[mCompoCount - 1];
    } break;

    default: break;
    }

    mSize = aSize;

    Global::Functions& ggl = Global::functions();
    ggl.glGenTextures(1, &mId);
    ggl.glBindTexture(GL_TEXTURE_1D, mId);

    ggl.glTexImage1D(GL_TEXTURE_1D, 0, internalFormat, aSize,
                     0, mFormat, mType, aData);
    ggl.glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ggl.glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ggl.glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    ggl.glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    ggl.glBindTexture(GL_TEXTURE_1D, 0);

    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
}

void ComputeTexture1D::update(const void* aData)
{
    update(aData, 0, mSize);
}

void ComputeTexture1D::update(const void* aData, GLint aOffset, GLsizei aSize)
{
    XC_PTR_ASSERT(aData);
    XC_ASSERT(aSize > 0);
    XC_ASSERT(mId != 0);
    XC_ASSERT(aOffset + aSize <= mSize);

    Global::Functions& ggl = Global::functions();
    ggl.glBindTexture(GL_TEXTURE_1D, mId);
    ggl.glTexSubImage1D(GL_TEXTURE_1D, 0, aOffset, aSize, mFormat, mType, aData);
    ggl.glBindTexture(GL_TEXTURE_1D, 0);

    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
}

void ComputeTexture1D::destroy()
{
    if (mId != 0)
    {
        Global::Functions& ggl = Global::functions();
        ggl.glDeleteTextures(1, &mId);
        mId = 0;
        XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
    }
}

//-------------------------------------------------------------------------------------------------
ComputeTexture1DList::ComputeTexture1DList(ComputeTexture1D::CompoType aCompoType, int aCompoCount)
    : mCompoType(aCompoType)
    , mCompoCount(aCompoCount)
    , mList()
    , mSize()
{
}

ComputeTexture1DList::~ComputeTexture1DList()
{
    destroy();
}

void ComputeTexture1DList::reset(int aCount, GLsizei aSize, const void* aData)
{
    const int prevCount = count();

    if (aCount > 0)
    {
        XC_ASSERT(aSize > 0);
        if (mSize != aSize)
        {
            destroy();
            for (int i = 0; i < aCount; ++i)
            {
                auto texture = new ComputeTexture1D(mCompoType, mCompoCount);
                texture->create(aData, aSize);
                mList.push_back(texture);
            }
        }
        else
        {
            if (prevCount < aCount)
            {
                for (int i = 0; i < aCount - prevCount; ++i)
                {
                    auto texture = new ComputeTexture1D(mCompoType, mCompoCount);
                    texture->create(aData, aSize);
                    mList.push_back(texture);
                }
            }
            else
            {
                for (int i = 0; i < prevCount - aCount; ++i)
                {
                    auto texture = mList.back();
                    delete texture;
                    mList.pop_back();
                }
            }
        }
    }
    else
    {
        destroy();
    }
    mSize = aSize;
}

void ComputeTexture1DList::destroy()
{
    if (!mList.empty())
    {
        qDeleteAll(mList);
        mList.clear();
        mSize = 0;
    }
}

ComputeTexture1D& ComputeTexture1DList::at(int aIndex)
{
    XC_ASSERT(0 <= aIndex && aIndex < count());
    return *mList[aIndex];
}

const ComputeTexture1D& ComputeTexture1DList::at(int aIndex) const
{
    XC_ASSERT(0 <= aIndex && aIndex < count());
    return *mList[aIndex];
}

} // namespace gl
