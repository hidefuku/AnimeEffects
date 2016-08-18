#ifndef GL_VECTOR3
#define GL_VECTOR3

#include <QGL>
#include <QVector2D>
#include <QVector3D>
#include <QPoint>
#include "gl/Vector2.h"

namespace gl
{

struct Vector3
{
    static Vector3 make(GLfloat aX, GLfloat aY, GLfloat aZ) { Vector3 r; r.set(aX, aY, aZ); return r; }
    inline void setZero() { x = 0.0f; y = 0.0f; z = 0.0f; }
    inline void set(GLfloat aX, GLfloat aY, GLfloat aZ) { x = aX; y = aY; z = aZ; }
    inline void set(const QVector3D& aV) { x = aV.x(); y = aV.y(); z = aV.z(); }
    inline void set(const QPoint& aV, GLfloat aZ = 0.0f) { x = aV.x(); y = aV.y(); z = aZ; }
    GLfloat x;
    GLfloat y;
    GLfloat z;
    QVector3D pos() const { return QVector3D(x, y, z); }
    QVector2D pos2D() const { return QVector2D(x, y); }
    Vector2 vec2() const { return Vector2::make(x, y); }


    Vector3 operator +(const Vector3& aRhs) const
    {
        Vector3 result = { x + aRhs.x, y + aRhs.y, z + aRhs.z }; return result;
    }

    Vector3 operator -(const Vector3& aRhs) const
    {
        Vector3 result = { x - aRhs.x, y - aRhs.y, z - aRhs.z }; return result;
    }

    Vector3 operator *(const Vector3& aRhs) const
    {
        Vector3 result = { x * aRhs.x, y * aRhs.y, z * aRhs.z }; return result;
    }

    Vector3 operator *(float aRhs) const
    {
        Vector3 result = { x * aRhs, y * aRhs, z * aRhs }; return result;
    }

    Vector3 operator /(const Vector3& aRhs) const
    {
        Vector3 result = { x / aRhs.x, y / aRhs.y, z / aRhs.z }; return result;
    }

    Vector3 operator /(float aRhs) const
    {
        Vector3 result = { x / aRhs, y / aRhs, z / aRhs }; return result;
    }

    Vector3& operator +=(const Vector3& aRhs) { x += aRhs.x; y += aRhs.y; z += aRhs.z; return *this; }
    Vector3& operator -=(const Vector3& aRhs) { x -= aRhs.x; y -= aRhs.y; z -= aRhs.z; return *this; }
    Vector3& operator *=(float aRhs) { x *= aRhs; y *= aRhs; z *= aRhs; return *this; }
    Vector3& operator /=(float aRhs) { x /= aRhs; y /= aRhs; z /= aRhs; return *this; }
};

} // namespace gl

#endif // GL_VECTOR3

