#ifndef GL_VECTOR2
#define GL_VECTOR2

#include <QGL>
#include <QVector2D>

namespace gl
{

struct Vector2
{
    static Vector2 make(GLfloat aX, GLfloat aY) { Vector2 r; r.set(aX, aY); return r; }
    static Vector2 make(const QVector2D& aV) { Vector2 r; r.set(aV.x(), aV.y()); return r; }

    inline void setZero() { x = 0.0f; y = 0.0f; }
    inline void set(GLfloat aX, GLfloat aY) { x = aX; y = aY; }
    inline void set(const QVector2D& aV) { x = aV.x(); y = aV.y(); }
    inline bool isZero() const { return x == 0.0f && y == 0.0f; }
    QVector2D pos() const { return QVector2D(x, y); }

    Vector2 operator +(const Vector2& aRhs) const
    {
        Vector2 result = { x + aRhs.x, y + aRhs.y }; return result;
    }

    Vector2 operator -(const Vector2& aRhs) const
    {
        Vector2 result = { x - aRhs.x, y - aRhs.y }; return result;
    }

    Vector2 operator *(const Vector2& aRhs) const
    {
        Vector2 result = { x * aRhs.x, y * aRhs.y }; return result;
    }

    Vector2 operator *(float aRhs) const
    {
        Vector2 result = { x * aRhs, y * aRhs }; return result;
    }

    Vector2 operator /(const Vector2& aRhs) const
    {
        Vector2 result = { x / aRhs.x, y / aRhs.y }; return result;
    }

    Vector2 operator /(float aRhs) const
    {
        Vector2 result = { x / aRhs, y / aRhs }; return result;
    }

    Vector2& operator +=(const Vector2& aRhs) { x += aRhs.x; y += aRhs.y; return *this; }
    Vector2& operator -=(const Vector2& aRhs) { x -= aRhs.x; y -= aRhs.y; return *this; }
    Vector2& operator *=(float aRhs) { x *= aRhs; y *= aRhs; return *this; }
    Vector2& operator /=(float aRhs) { x /= aRhs; y /= aRhs; return *this; }

    GLfloat x;
    GLfloat y;
};

} // namespace gl

#endif // GL_VECTOR2

