#include "gl/Global.h"
#include "core/DestinationTexturizer.h"

namespace
{
static const int kAttachmentId = 0;
}

namespace core
{

DestinationTexturizer::DestinationTexturizer()
    : mFramebuffer()
    , mTexture()
    , mShader()
{
    mFramebuffer.reset(new gl::Framebuffer());
    mTexture.reset(new gl::Texture());

    createShader();
}

void DestinationTexturizer::resize(const QSize& aSize)
{
    mTexture->destroy();
    mFramebuffer.reset();

    // create framebuffer
    mFramebuffer.reset(new gl::Framebuffer());

    // create texture
    mTexture->create(aSize);
    mTexture->setFilter(GL_NEAREST);
    mTexture->setWrap(GL_CLAMP_TO_EDGE);

    // attach textures
    mFramebuffer->setColorAttachment(kAttachmentId, mTexture->id());
    XC_ASSERT(mFramebuffer->isComplete());
}

void DestinationTexturizer::update(
        GLuint aFramebuffer, GLuint aFrameTexture, const QMatrix4x4& aViewMatrix,
        const LayerMesh& aMesh, gl::BufferObject& aPositions)
{
    XC_ASSERT(mTexture->size().isValid());

    auto& ggl = gl::Global::functions();

    // setup drawbuffers
    const GLenum attachments[] = { GL_COLOR_ATTACHMENT0 };
    ggl.glDrawBuffers(1, attachments);

    // bind framebuffer
    mFramebuffer->bind();

    // bind textures
    ggl.glActiveTexture(GL_TEXTURE0);
    ggl.glBindTexture(GL_TEXTURE_2D, aFrameTexture);

    {
        mShader.bind();

        mShader.setAttributeBuffer("inPosition", aPositions, GL_FLOAT, 3);

        mShader.setUniformValue("uViewMatrix", aViewMatrix);
        mShader.setUniformValue("uScreenSize", QSizeF(mTexture->size()));
        mShader.setUniformValue("uDestTexture", 0);

        ggl.glDrawElements(
                    aMesh.primitiveMode(), aMesh.indexCount(),
                    GL_UNSIGNED_INT, aMesh.indices());

        mShader.release();
    }
    // unbind texture
    ggl.glBindTexture(GL_TEXTURE_2D, 0);

    // release framebuffer
    mFramebuffer->release();

    // bind default framebuffer
    ggl.glBindFramebuffer(GL_FRAMEBUFFER, aFramebuffer);

    ggl.glFlush();
}

void DestinationTexturizer::createShader()
{
    auto shader = &mShader;

    gl::ExtendShader source;
    if (!source.openFromFile("./data/shader/PartialScreenCopying.glslex"))
    {
        XC_FATAL_ERROR("FileIO Error", "Failed to open shader file.",
                       source.log());
    }
    if (!source.resolveVariation())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to resolve shader variation.",
                       source.log());
    }

    if (!shader->setAllSource(source))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile shader.",
                       shader->log());
    }

    if (!shader->link())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.",
                       shader->log());
    }
}

} // namespace core
