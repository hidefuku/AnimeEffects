#ifndef GL_FRAMEBUFFER_H
#define GL_FRAMEBUFFER_H

#include <array>
#include <QGL>
#include "util/NonCopyable.h"

namespace gl
{

class Framebuffer : private util::NonCopyable
{
public:
    Framebuffer();
    ~Framebuffer();

    void setColorAttachment(int aAttachIndex, GLuint aTexture2D);
    bool isComplete() const;

    void bind();
    void release();

    explicit operator bool() const { return mId != 0; }
    GLuint id() const { return mId; }
    GLuint colorAttachment(int aAttachIndex) const;

private:
    GLuint mId;
    std::array<GLuint, 8> mColors;
};

} // namespace gl

#endif // GL_FRAMEBUFFER_H
