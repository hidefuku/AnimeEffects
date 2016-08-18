#ifndef GL_BUFFEROBJECT_H
#define GL_BUFFEROBJECT_H

#include <QGL>
#include "util/NonCopyable.h"

namespace gl
{

class BufferObject : private util::NonCopyable
{
public:
    BufferObject(GLenum aType);
    ~BufferObject();

    template<typename tValue>
    void resetData(int aDataCount, GLenum aUsage, const tValue* aDataPtr = NULL)
    {
        mDataCount = aDataCount;
        resetDataImpl(sizeof(tValue) * aDataCount, aDataPtr, aUsage);
    }

    template<typename tType>
    void copyFrom(const BufferObject& aFrom)
    {
        copyFromImpl(aFrom, sizeof(tType));
    }

    explicit operator bool() const { return mId != 0; }
    int dataCount() const { return mDataCount; }
    GLenum type() const { return mType; }
    GLuint id() const { return mId; }

    void bind();
    void release();

private:
    void resetDataImpl(GLsizeiptr aSize, const GLvoid* aData, GLenum aUsage);
    void copyFromImpl(const BufferObject& aFrom, size_t aTypeSize);

    GLenum mType;
    GLuint mId;
    int mDataCount;
};

} // namespace gl

#endif // GL_BUFFEROBJECT_H
