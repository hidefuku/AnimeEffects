#ifndef GL_VECTOR4
#define GL_VECTOR4

#include <QGL>
#include <QVector4D>

namespace gl
{

struct Vector4
{
    inline void setZero() { x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f; }
    inline void set(GLfloat aX, GLfloat aY, GLfloat aZ, GLfloat aW) { x = aX; y = aY; z = aZ; w = aW; }
    inline void set(const QVector4D& aV) { x = aV.x(); y = aV.y(); z = aV.z(); w = aV.w(); }
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat w;
    QVector4D pos() const { return QVector4D(x, y, z, w); }
};

} // namespace gl

#endif // GL_VECTOR4

