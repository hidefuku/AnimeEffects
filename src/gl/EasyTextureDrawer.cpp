#include "gl/EasyTextureDrawer.h"
#include "gl/Global.h"
#include "gl/Util.h"

namespace gl
{

//-------------------------------------------------------------------------------------------------
EasyTextureDrawer::EasyTextureDrawer()
    : mShader()
    , mIndices()
{
}

bool EasyTextureDrawer::init()
{
    static const char* kVertexShaderText =
            "#version 330 \n"
            "in vec2 inPosition;"
            "in vec2 inTexCoord;"
            "out vec2 vTexCoord;"
            "void main(void){"
            "  gl_Position = vec4(inPosition, 0.0, 1.0);"
            "  vTexCoord = inTexCoord;"
            "}";
    static const char* kFragmentShaderText =
            "#version 330 \n"
            "uniform sampler2D uTexture0;"
            "in vec2 vTexCoord;"
            "layout(location = 0, index = 0) out vec4 oFragColor;"
            "void main(void){"
            "  oFragColor = texture(uTexture0, vTexCoord);"
            "}";

    if (!mShader.setVertexSource(QString(kVertexShaderText)))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile vertex shader.",
                       mShader.log());
        return false;
    }
    if (!mShader.setFragmentSource(QString(kFragmentShaderText)))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile fragment shader.",
                       mShader.log());
        return false;
    }
    if (!mShader.link())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.",
                       mShader.log());
        return false;
    }

    {
        static const GLuint kIndices[4] = { 0, 1, 3, 2 };
        mIndices.reset(new gl::BufferObject(GL_ELEMENT_ARRAY_BUFFER));
        mIndices->resetData(4, GL_STATIC_DRAW, kIndices);
    }

    return true;
}
void EasyTextureDrawer::draw(
        GLuint aTexture, const QRectF& aPosRect, const QSize& aPosCoord,
        const QRectF& aTexRect, const QSize& aTexCoord)
{
    std::array<gl::Vector2, 4> positions;
    {
        const float x1 = -1.0f + 2.0f * (aPosRect.left()   / aPosCoord.width());
        const float x2 = -1.0f + 2.0f * (aPosRect.right()  / aPosCoord.width());
        const float y1 =  1.0f - 2.0f * (aPosRect.top()    / aPosCoord.height());
        const float y2 =  1.0f - 2.0f * (aPosRect.bottom() / aPosCoord.height());
        positions[0].set(x1, y1);
        positions[1].set(x1, y2);
        positions[2].set(x2, y2);
        positions[3].set(x2, y1);
    }

    std::array<gl::Vector2, 4> texCoords;
    {
        const float u1 =  aTexRect.left()   / aTexCoord.width();
        const float u2 =  aTexRect.right()  / aTexCoord.width();
        const float v1 = -aTexRect.top()    / aTexCoord.height() + 1.0f;
        const float v2 = -aTexRect.bottom() / aTexCoord.height() + 1.0f;
        texCoords[0].set(u1, v1);
        texCoords[1].set(u1, v2);
        texCoords[2].set(u2, v2);
        texCoords[3].set(u2, v1);
    }
    draw(aTexture, positions, texCoords);
}

void EasyTextureDrawer::draw(GLuint aTexture)
{
    std::array<gl::Vector2, 4> positions;
    positions[0].set(-1.0f, -1.0f);
    positions[1].set(-1.0f,  1.0f);
    positions[2].set( 1.0f,  1.0f);
    positions[3].set( 1.0f, -1.0f);

    std::array<gl::Vector2, 4> texCoords;
    texCoords[0].set(0.0f, 0.0f);
    texCoords[1].set(0.0f, 1.0f);
    texCoords[2].set(1.0f, 1.0f);
    texCoords[3].set(1.0f, 0.0f);

    draw(aTexture, positions, texCoords);
}

void EasyTextureDrawer::draw(GLuint aTexture,
          const std::array<QVector2D, 4>& aPosQuad, const QSize& aPosCoord,
          const std::array<QVector2D, 4>& aTexQuad, const QSize& aTexCoord)
{
    std::array<gl::Vector2, 4> positions;
    std::array<gl::Vector2, 4> texCoords;
    const QVector2D posdiv(aPosCoord.width(), aPosCoord.height());
    const QVector2D texdiv(aTexCoord.width(), aTexCoord.height());

    for (int i = 0; i < 4; ++i)
    {
        auto pos = aPosQuad[i];
        auto tex = aTexQuad[i];
        positions[i].set(-1.0f + 2.0f * pos.x() / posdiv.x(),
                         1.0f - 2.0f * pos.y() / posdiv.y());
        texCoords[i].set(tex.x() / texdiv.x(),
                         -tex.y() / texdiv.y() + 1.0f);
    }
    draw(aTexture, positions, texCoords);

}

void EasyTextureDrawer::draw(
        GLuint aTexture,
        const std::array<gl::Vector2, 4>& aPositions,
        const std::array<gl::Vector2, 4>& aTexCoords)
{
    Global::Functions& ggl = Global::functions();

    Util::resetRenderState();

    ggl.glActiveTexture(GL_TEXTURE0);
    ggl.glBindTexture(GL_TEXTURE_2D, aTexture);
    {
        mShader.bind();
        mShader.setAttributeArray("inPosition", aPositions.data(), 4);
        mShader.setAttributeArray("inTexCoord", aTexCoords.data(), 4);
        mShader.setUniformValue("uTexture0", 0);
        Util::drawElements(GL_TRIANGLE_STRIP, GL_UNSIGNED_INT, *mIndices);
        mShader.release();
    }
    ggl.glActiveTexture(GL_TEXTURE0);
    ggl.glBindTexture(GL_TEXTURE_2D, 0);

    GL_CHECK_ERROR();
}

} // namespace gl

