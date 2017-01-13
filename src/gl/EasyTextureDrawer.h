#ifndef GL_EASYTEXTUREDRAWER_H
#define GL_EASYTEXTUREDRAWER_H

#include <array>
#include "util/NonCopyable.h"
#include "gl/EasyShaderProgram.h"
#include "gl/BufferObject.h"

namespace gl
{

class EasyTextureDrawer : private util::NonCopyable
{
public:
    EasyTextureDrawer();
    bool init();
    void draw(GLuint aTexture,
              const std::array<gl::Vector2, 4>& aPositions,
              const std::array<gl::Vector2, 4>& aTexCoords);
    void draw(GLuint aTexture,
              const std::array<QVector2D, 4>& aPosQuad, const QSize& aPosCoord,
              const std::array<QVector2D, 4>& aTexQuad, const QSize& aTexCoord);
    void draw(GLuint aTexture);
    void draw(GLuint aTexture, const QRectF& aPosRect, const QSize& aPosCoord,
              const QRectF& aTexRect, const QSize& aTexCoord);

private:
    gl::EasyShaderProgram mShader;
    QScopedPointer<gl::BufferObject> mIndices;
};

} // namespace gl

#endif // GL_EASYTEXTUREDRAWER_H
