#ifndef GL_UTIL_H
#define GL_UTIL_H

#include <QGL>
#include <QSize>

namespace gl
{

class Util
{
public:
    static void clearColorBuffer(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
    static void setViewportAsActualPixels(const QSize& aSize);
    static void resetRenderState();
    static GLuint findTextureFromColorAttachment0();

private:
    Util() {}
};

} // namespace gl

#endif // GL_UTIL_H
