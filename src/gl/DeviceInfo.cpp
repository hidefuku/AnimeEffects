#include "XC.h"
#include "gl/DeviceInfo.h"
#include "gl/Global.h"

namespace
{
static bool sIsDeviceInfoInitialized = false;
static gl::DeviceInfo sDeviceInfo;
} // namespace

namespace gl
{

const DeviceInfo& DeviceInfo::instance()
{
    XC_PTR_ASSERT(sIsDeviceInfoInitialized);
    return sDeviceInfo;
}

void DeviceInfo::createInstance()
{
    Global::makeCurrent();
    auto& ggl = Global::functions();

    DeviceInfo info;
    ggl.glGetIntegerv(GL_MAX_TEXTURE_SIZE, &info.maxTextureSize);
    ggl.glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &info.maxRenderBufferSize);

    sDeviceInfo = info;
    sIsDeviceInfoInitialized = true;

#if 0
    GLint val = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &val);
    qDebug() << "max vertex attribs" << val;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &val);
    qDebug() << "max varying vectors" << val;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &val);
    qDebug() << "max vertex uniform vectors" << val;
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &val);
    qDebug() << "max fragment uniform vectors" << val;
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &val);
    qDebug() << "max vertex texture image units" << val;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &val);
    qDebug() << "max texture image units" << val;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &val);
    qDebug() << "max texture size" << val;
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &val);
    qDebug() << "max cube map texture size" << val;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &val);
    qDebug() << "max renderbuffer size" << val;
    GLint dims[2] = {};
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);
    qDebug() << "max viewport dims" << dims[0] << dims[1];

    //glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
    //glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxAttach);
#endif
}

DeviceInfo::DeviceInfo()
    : maxTextureSize(0)
    , maxRenderBufferSize(0)
{
}

bool DeviceInfo::isValid() const
{
    return maxTextureSize > 0 && maxRenderBufferSize > 0;
}

} // namespace gl

