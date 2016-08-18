#include "gl/Global.h"
#include "gl/Util.h"
#include "core/ClippingFrame.h"

namespace
{
static const int kAttachmentId = 0;
}

namespace core
{

ClippingFrame::ClippingFrame()
    : mFramebuffer()
    , mTexture()
    , mClippingId(0)
    , mSingulationShader()
    , mRenderStamp()
{
    mFramebuffer.reset(new gl::Framebuffer());
    mTexture.reset(new gl::Texture());

    createSingulationShader();
}

void ClippingFrame::resize(const QSize& aSize)
{
    auto format = GL_RG_INTEGER;
    auto internalFormat = GL_RG8UI;

    mFramebuffer.reset();
    mFramebuffer.reset(new gl::Framebuffer());

    // create texture
    mTexture->create(aSize, nullptr, format, internalFormat, GL_UNSIGNED_BYTE);
    mTexture->setFilter(GL_NEAREST);
    mTexture->setWrap(GL_CLAMP_TO_EDGE);

    // attach textures
    mFramebuffer->setColorAttachment(kAttachmentId, mTexture->id());
    XC_ASSERT(mFramebuffer->isComplete());
}

void ClippingFrame::clearTexture()
{
    if (mTexture->id() == 0) return;

    static const GLuint clearColorU[] = { 0, 0, 0, 0 };
    auto& ggl = gl::Global::functions();

    mFramebuffer->bind();
    setupDrawBuffers();

    gl::Util::setViewportAsActualPixels(mTexture->size());
    gl::Util::resetRenderState();
    ggl.glClearBufferuiv(GL_COLOR, kAttachmentId, clearColorU);

    mFramebuffer->release();

#if 0
    {
        auto frameSize = mTexture->size();
        QScopedArrayPointer<uint8> data(new uint8[frameSize.width() * frameSize.height() * 2]);
        ggl.glBindTexture(GL_TEXTURE_2D, mTexture->id());
        ggl.glGetTexImage(GL_TEXTURE_2D, 0, GL_RG_INTEGER, GL_UNSIGNED_BYTE, data.data());
        ggl.glBindTexture(GL_TEXTURE_2D, 0);
        ggl.glFinish();

        for (int i = 0; i < frameSize.width() * frameSize.height(); ++i)
        {
            if (data[i * 2] != 0)
            {
                qDebug() << "init" << (int)(data[i * 2]);
                break;
            }
        }
    }
#endif

    XC_ASSERT(gl::Global::functions().glGetError() == GL_NO_ERROR);
}

void ClippingFrame::createSingulationShader()
{
    static const char* kVertexShaderText =
            "#version 130 \n"
            "in vec2 inPosition;"
            "in vec2 inTexCoord;"
            "out vec2 vTexCoord;"
            "void main() {"
            "  gl_Position = vec4(inPosition, 0.0, 1.0);"
            "  vTexCoord = inTexCoord;"
            "}";

    static const char* kFragmentShaderText =
            "#version 130 \n"
            "uniform uint uSingulation;"
            "uniform usampler2D uTexture;"
            "in vec2 vTexCoord;"
            "out uvec2 oClip;"
            "void main() {"
            "  uvec2 current = texture(uTexture, vTexCoord).xy;"
            "  if (uSingulation == current.x) {"
            "    oClip = uvec2(1, current.y);"
            "  }"
            "  else {"
            "    oClip = uvec2(0, 0);"
            "  }"
            "}";

    auto& shader = mSingulationShader;
    auto& ggl = gl::Global::functions();

    if (!shader.setVertexSource(QString(kVertexShaderText)))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile vertex shader.",
                       shader.log());
    }
    if (!shader.setFragmentSource(QString(kFragmentShaderText)))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile fragment shader.",
                       shader.log());
    }

    ggl.glBindFragDataLocation(shader.id(), 0, "oClip");

    if (!shader.link())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.",
                       shader.log());
    }
}

void ClippingFrame::singulate(uint8 aId)
{
    if (mTexture->id() == 0) return;

    static const GLuint kIndices[4] = { 0, 1, 2, 3 };
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
    auto& ggl = gl::Global::functions();
    auto& shader = mSingulationShader;
    auto textureId = mTexture->id();
    auto textureSize = mTexture->size();

    mFramebuffer->bind();
    setupDrawBuffers();
    gl::Util::setViewportAsActualPixels(textureSize);
    gl::Util::resetRenderState();

    ggl.glEnable(GL_TEXTURE_2D);
    ggl.glActiveTexture(GL_TEXTURE0);
    ggl.glBindTexture(GL_TEXTURE_2D, textureId);

    shader.bind();
    shader.setAttributeArray("inPosition", positions.data());
    shader.setAttributeArray("inTexCoord", texCoords.data());
    shader.setUniformValue("uSingulation", (GLuint)aId);
    shader.setUniformValue("uTexture0", 0);
    ggl.glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, kIndices);
    shader.release();

    ggl.glActiveTexture(GL_TEXTURE0);
    ggl.glBindTexture(GL_TEXTURE_2D, 0);
    ggl.glDisable(GL_TEXTURE_2D);

    mFramebuffer->release();
    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
}

void ClippingFrame::bind()
{
    mFramebuffer->bind();
}

void ClippingFrame::release()
{
    mFramebuffer->release();
}

void ClippingFrame::setupDrawBuffers()
{
    const GLenum attachments[] = { GL_COLOR_ATTACHMENT0 };
    gl::Global::functions().glDrawBuffers(1, attachments);
}

uint8 ClippingFrame::forwardClippingId()
{
    if (mClippingId < 255)
    {
        ++mClippingId;
    }
    else
    {
        mClippingId = 1;
        clearTexture();
    }
    return mClippingId;
}

/*
void ClippingFrame::drawClipping(gl::BufferObject& aPositions)
{
    {
        std::array<gl::Vector2, 4> positions;
        positions[0].set(-1.0f, -1.0f);
        positions[1].set(-1.0f,  1.0f);
        positions[2].set( 1.0f,  1.0f);
        positions[3].set( 1.0f, -1.0f);

        static const GLuint kIndices[4] = { 0, 1, 2, 3 };
        shader.bind();
        shader.setAttributeArray("inPosition", positions.data());
        ggl.glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, kIndices);
        shader.release();
    }
}
*/

} // namespace core
