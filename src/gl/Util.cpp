#include "XC.h"
#include "gl/Util.h"
#include "gl/Global.h"

namespace gl
{

void Util::clearColorBuffer(GLclampf r, GLclampf g, GLclampf b, GLclampf a)
{
    Global::Functions& ggl = Global::functions();
    ggl.glClearColor(r, g, b, a);
    ggl.glClear(GL_COLOR_BUFFER_BIT);
}

void Util::setViewportAsActualPixels(const QSize& aSize)
{
    Global::Functions& ggl = Global::functions();
    ggl.glViewport(0, 0, aSize.width(), aSize.height());
}

void Util::resetRenderState()
{
    Global::Functions& ggl = Global::functions();

    ggl.glDisable(GL_BLEND);
    ggl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ggl.glDisable(GL_DEPTH_TEST);
    ggl.glDisable(GL_STENCIL_TEST);
    ggl.glDisable(GL_SCISSOR_TEST);
    ggl.glDisable(GL_CULL_FACE);
    ggl.glDisable(GL_MULTISAMPLE);
    ggl.glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    ggl.glDisable(GL_SAMPLE_ALPHA_TO_ONE);
    ggl.glDisable(GL_SAMPLE_COVERAGE);
    ggl.glDisable(GL_POLYGON_OFFSET_FILL);
    ggl.glDisable(GL_DITHER);
    ggl.glDisable(GL_COLOR_LOGIC_OP);

    // legacy options (for gl removed)
#if 0
    //ggl.glDisable(GL_ALPHA_TEST);
    //ggl.glDisable(GL_TEXTURE_2D);
    //ggl.glDisable(GL_LIGHTING);
    //ggl.glDisable(GL_FOG);
    //ggl.glDisable(GL_POLYGON_SMOOTH);
    //ggl.glDisable(GL_LINE_SMOOTH);
    //ggl.glDisable(GL_POINT_SMOOTH);
    //ggl.glDisable(GL_NORMALIZE);
    //ggl.glDisable(GL_RESCALE_NORMAL);
    //ggl.glDisable(GL_COLOR_MATERIAL);
#endif
    GL_CHECK_ERROR();

    //ggl.glColor4f(1.0, 1.0, 1.0, 1.0);

    //ggl.glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    //ggl.glDisableClientState(GL_COLOR_ARRAY);
}

GLuint Util::findTextureFromColorAttachment0()
{
    static const GLenum kTarget = GL_FRAMEBUFFER;
    static const GLenum kAttach = GL_COLOR_ATTACHMENT0;
    gl::Global::Functions& ggl = gl::Global::functions();

    GLint value = 0;
    ggl.glGetFramebufferAttachmentParameteriv(
                kTarget, kAttach, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &value);

    if ((GLenum)value == GL_TEXTURE)
    {
        ggl.glGetFramebufferAttachmentParameteriv(
                    kTarget, kAttach, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &value);
        return (GLuint)value;
    }
    return 0;
}

} // namespace gl
