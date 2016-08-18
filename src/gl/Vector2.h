#ifndef GL_VECTOR2
#define GL_VECTOR2

#include <QGL>
#include <QVector2D>

namespace gl
{

struct Vector2
{
    static Vector2 make(GLfloat aX, GLfloat aY) { Vector2 r; r.set(aX, aY); return r; }

    inline void setZero() { x = 0.0f; y = 0.0f; }
    inline void set(GLfloat aX, GLfloat aY) { x = aX; y = aY; }
    inline void set(const QVector2D& aV) { x = aV.x(); y = aV.y(); }
    inline bool isZero() const { return x == 0.0f && y == 0.0f; }
    GLfloat x;
    GLfloat y;
};

} // namespace gl

#endif // GL_VECTOR2

