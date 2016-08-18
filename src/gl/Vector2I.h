#ifndef GL_VECTOR2I
#define GL_VECTOR2I

#include <QGL>
#include <QPoint>

namespace gl
{

struct Vector2I
{
    inline void setZero() { x = 0; y = 0; }
    inline void set(GLint aX, GLint aY) { x = aX; y = aY; }
    inline void set(const QPoint& aV) { x = aV.x(); y = aV.y(); }
    GLint x;
    GLint y;
};

} // namespace gl
#endif // GL_VECTOR2I

