#include "XC.h"
#include "gl/BufferObject.h"
#include "gl/Global.h"

namespace gl
{

BufferObject::BufferObject(GLenum aType)
    : mType(aType)
    , mId(0)
    , mTypeSize(0)
    , mUsingDataCount(0)
    , mBufferCount(0)
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
    if (mTypeSize == aTypeSize && mBufferCount == aDataCount && mUsage == aUsage)
    {
        mUsingDataCount = aDataCount;
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
        mUsingDataCount = aDataCount;
        mBufferCount = aDataCount;
        mUsage = aUsage;

        ggl.glBindBuffer(mType, mId);
        ggl.glBufferData(mType, aTypeSize * aDataCount, aData, aUsage);
        ggl.glBindBuffer(mType, 0);
        GL_CHECK_ERROR();
    }
}

void BufferObject::fastResize(int aDataCount)
{
    XC_ASSERT((bool)*this);
    if (mBufferCount < aDataCount)
    {
        resetRawData(mTypeSize, nullptr, aDataCount, mUsage);
    }
    else
    {
        mUsingDataCount = aDataCount;
    }
}

void BufferObject::copyFrom(const BufferObject& aFrom)
{
    XC_MSG_ASSERT(mUsingDataCount == aFrom.dataCount(),
                  "data count: %d, %d", mUsingDataCount, aFrom.dataCount());
    XC_ASSERT(mTypeSize == aFrom.typeSize());
    XC_ASSERT(mId != 0 && aFrom.id() != 0);

    Global::Functions& ggl = Global::functions();

    ggl.glBindBuffer(mType, mId);
    ggl.glBindBuffer(aFrom.type(), aFrom.id());
    ggl.glCopyBufferSubData(aFrom.type(), mType, 0, 0, mTypeSize * mUsingDataCount);
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
