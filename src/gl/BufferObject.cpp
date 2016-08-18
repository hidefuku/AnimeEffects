#include "XC.h"
#include "gl/BufferObject.h"
#include "gl/Global.h"

namespace gl
{

BufferObject::BufferObject(GLenum aType)
    : mType(aType)
    , mId(0)
    , mDataCount(0)
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

void BufferObject::resetDataImpl(GLsizeiptr aSize, const GLvoid* aData, GLenum aUsage)
{
    Global::Functions& ggl = Global::functions();
    ggl.glBindBuffer(mType, mId);
    ggl.glBufferData(mType, aSize, aData, aUsage);
    ggl.glBindBuffer(mType, 0);
    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
}

void BufferObject::copyFromImpl(const BufferObject& aFrom, size_t aTypeSize)
{
    XC_ASSERT(mDataCount == aFrom.dataCount());
    XC_ASSERT(mId != 0 && aFrom.id() != 0);

    Global::Functions& ggl = Global::functions();

    ggl.glBindBuffer(mType, mId);
    ggl.glBindBuffer(aFrom.type(), aFrom.id());
    ggl.glCopyBufferSubData(aFrom.type(), mType, 0, 0, aTypeSize * mDataCount);
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
