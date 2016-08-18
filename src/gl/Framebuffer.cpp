#include "XC.h"
#include "gl/Framebuffer.h"
#include "gl/Global.h"

namespace gl
{

Framebuffer::Framebuffer()
    : mId(0)
    , mColors()
{
    Global::functions().glGenFramebuffers(1, &mId);
}

Framebuffer::~Framebuffer()
{
    if (mId != 0)
    {
        Global::functions().glDeleteFramebuffers(1, &mId);
    }
}

void Framebuffer::setColorAttachment(int aAttachIndex, GLuint aTexture2D)
{
    GLenum attach = GL_COLOR_ATTACHMENT0;
    switch (aAttachIndex)
    {
    case 0: attach = GL_COLOR_ATTACHMENT0; break;
    case 1: attach = GL_COLOR_ATTACHMENT1; break;
    case 2: attach = GL_COLOR_ATTACHMENT2; break;
    case 3: attach = GL_COLOR_ATTACHMENT3; break;
    case 4: attach = GL_COLOR_ATTACHMENT4; break;
    case 5: attach = GL_COLOR_ATTACHMENT5; break;
    case 6: attach = GL_COLOR_ATTACHMENT6; break;
    case 7: attach = GL_COLOR_ATTACHMENT7; break;
    default: XC_ASSERT(0); break;
    }

    Global::Functions& ggl = Global::functions();

    ggl.glBindFramebuffer(GL_FRAMEBUFFER, mId);
    ggl.glFramebufferTexture2D(
                GL_FRAMEBUFFER, attach,
                GL_TEXTURE_2D, aTexture2D, 0);
    ggl.glBindFramebuffer(GL_FRAMEBUFFER, 0);

    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);

    mColors[aAttachIndex] = aTexture2D;
}

bool Framebuffer::isComplete() const
{
    Global::Functions& ggl = Global::functions();

    ggl.glBindFramebuffer(GL_FRAMEBUFFER, mId);
    auto attachResult = ggl.glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ggl.glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return attachResult == GL_FRAMEBUFFER_COMPLETE;
}

void Framebuffer::bind()
{
    Global::functions().glBindFramebuffer(GL_FRAMEBUFFER, mId);
}

void Framebuffer::release()
{
    Global::functions().glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint Framebuffer::colorAttachment(int aAttachIndex) const
{
    return mColors.at(aAttachIndex);
}

} // namespace gl

