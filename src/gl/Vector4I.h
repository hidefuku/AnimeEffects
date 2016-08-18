#ifndef GL_VECTOR4I
#define GL_VECTOR4I

#include <QGL>

namespace gl
{

struct Vector4I
{
    inline void setZero() { x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f; }
    inline void set(GLint aX, GLint aY, GLint aZ, GLint aW) { x = aX; y = aY; z = aZ; w = aW; }
    GLint x;
    GLint y;
    GLint z;
    GLint w;
};

} // namespace gl
#endif // GL_VECTOR4I

