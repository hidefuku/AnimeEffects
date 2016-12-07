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
        resetRawData(sizeof(tValue), aDataPtr, aDataCount, aUsage);
    }

    void resetRawData(GLsizeiptr aTypeSize, const GLvoid* aData,
                      int aDataCount, GLenum aUsage);

    void fastResize(int aDataCount);

    void copyFrom(const BufferObject& aFrom);

    explicit operator bool() const { return mId != 0; }
    int dataCount() const { return mUsingDataCount; }
    GLenum type() const { return mType; }
    GLuint id() const { return mId; }
    GLsizeiptr typeSize() const { return mTypeSize; }

    void bind();
    void release();

private:
    GLenum mType;
    GLuint mId;
    GLsizeiptr mTypeSize;
    int mUsingDataCount;
    int mBufferCount;
    GLenum mUsage;
};

} // namespace gl

#endif // GL_BUFFEROBJECT_H
