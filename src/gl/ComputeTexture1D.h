#ifndef GL_COMPUTETEXTURE1D_H
#define GL_COMPUTETEXTURE1D_H

#include <QGL>
#include "XC.h"
#include "util/NonCopyable.h"

namespace gl
{

class ComputeTexture1D : private util::NonCopyable
{
public:
    enum CompoType
    {
        CompoType_S32,
        CompoType_F32,
        CompoType_TERM
    };

    ComputeTexture1D(CompoType aCompoType, int aCompoCount);
    ~ComputeTexture1D();

    void create(const void* aData, GLsizei aSize);
    void update(const void* aData);
    void update(const void* aData, GLint aOffset, GLsizei aSize);
    void destroy();

    GLsizei size() const { return mSize; }
    GLuint id() const { return mId; }
    bool isValid() const { return mId != 0; }

private:
    CompoType mCompoType;
    int mCompoCount;
    GLuint mId;
    GLsizei mSize;
    GLenum mFormat;
    GLenum mType;
};

class ComputeTexture1DList
{
public:
    typedef QVector<ComputeTexture1D*> ListType;

    ComputeTexture1DList(ComputeTexture1D::CompoType aCompoType, int aCompoCount);
    ~ComputeTexture1DList();

    void reset(int aCount, GLsizei aSize, const void* aData = nullptr);
    void destroy();
    ComputeTexture1D& at(int aIndex);
    const ComputeTexture1D& at(int aIndex) const;

    int count() const { return mList.count(); }

private:
    ComputeTexture1D::CompoType mCompoType;
    int mCompoCount;
    ListType mList;
    GLsizei mSize;
};

} // namespace gl

#endif // GL_COMPUTETEXTURE1D_H
