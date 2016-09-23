#include "XC.h"
#include "gl/BufferObject.h"
#include "gl/Global.h"

namespace gl
{

BufferObject::BufferObject(GLenum aType)
    : mType(aType)
    , mId(0)
    , mTypeSize(0)
    , mDataCount(0)
    , mUsage()
{
    Global::functions().glGenBuffers(1, &mId);
}

BufferObject::~BufferObject()
{
    if (mId != 0)
    {
        Global::functions().glDeleteBuffers(1, &mId);
    }
}

void BufferObject::resetRawData(GLsizeiptr aTypeSize, const GLvoid* aData,
                                int aDataCount, GLenum aUsage)
{
    Global::Functions& ggl = Global::functions();
    if (mTypeSize == aTypeSize && mDataCount == aDataCount && mUsage == aUsage)
    {
        if (aData)
        {
            ggl.glBindBuffer(mType, mId);
            ggl.glBufferSubData(mType, 0, aTypeSize * aDataCount, aData);
            ggl.glBindBuffer(mType, 0);
            GL_CHECK_ERROR();
        }
    }
    else
    {
        mTypeSize = aTypeSize;
        mDataCount = aDataCount;
        mUsage = aUsage;

        ggl.glBindBuffer(mType, mId);
        ggl.glBufferData(mType, aTypeSize * aDataCount, aData, aUsage);
        ggl.glBindBuffer(mType, 0);
        GL_CHECK_ERROR();
    }
}

void BufferObject::copyFrom(const BufferObject& aFrom)
{
    XC_ASSERT(mDataCount == aFrom.dataCount());
    XC_ASSERT(mTypeSize == aFrom.typeSize());
    XC_ASSERT(mId != 0 && aFrom.id() != 0);

    Global::Functions& ggl = Global::functions();

    ggl.glBindBuffer(mType, mId);
    ggl.glBindBuffer(aFrom.type(), aFrom.id());
    ggl.glCopyBufferSubData(aFrom.type(), mType, 0, 0, mTypeSize * mDataCount);
    ggl.glBindBuffer(mType, 0);
    ggl.glBindBuffer(aFrom.type(), 0);
}

void BufferObject::bind()
{
    Global::functions().glBindBuffer(mType, mId);
}

void BufferObject::release()
{
    Global::functions().glBindBuffer(mType, 0);
}

} // namespace gl
