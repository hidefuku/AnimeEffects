#ifndef GL_UTIL_H
#define GL_UTIL_H

#include <QGL>
#include <QSize>
namespace gl { class BufferObject; }

namespace gl
{

class Util
{
public:
    static void clearColorBuffer(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
    static void setViewportAsActualPixels(const QSize& aSize);
    static void resetRenderState();
    static void setAbility(GLenum aState, bool aIsEnable);
    static GLuint findTextureFromColorAttachment0();
    static void drawElements(GLenum aPrimitive, GLenum aType, gl::BufferObject& aIndices);

private:
    Util() {}
};

} // namespace gl

#endif // GL_UTIL_H
