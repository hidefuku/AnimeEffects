#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H

#include <QGL>
#include "XC.h"
#include "util/NonCopyable.h"

namespace gl
{

class Texture : private util::NonCopyable
{
public:
    Texture();
    ~Texture();

    void create(
            const QSize& aSize,
            const uint8* aData = nullptr,
            GLenum aFormat = GL_RGBA,
            GLint aInternalFormat = GL_RGBA8,
            GLenum aChannelType = GL_UNSIGNED_BYTE);

    void setFilter(GLint aParam);
    void setWrap(GLint aParam, QColor aBorderColor = QColor(0, 0, 0, 0));

    void destroy();

    GLuint id() const { return mId; }
    QSize size() const { return mSize; }

private:
    GLuint mId;
    QSize mSize;
};

} // namespace gl

#endif // GL_TEXTURE_H
