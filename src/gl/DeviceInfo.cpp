#include "XC.h"
#include "gl/DeviceInfo.h"
#include "gl/Global.h"

namespace
{
static const gl::DeviceInfo* sDeviceInfoPtr;
} // namespace

namespace gl
{

const DeviceInfo& DeviceInfo::instance()
{
    XC_PTR_ASSERT(sDeviceInfoPtr);
    return *sDeviceInfoPtr;
}

void DeviceInfo::setInstance(const DeviceInfo* aInstance)
{
    sDeviceInfoPtr = aInstance;
}

bool DeviceInfo::validInstanceExists()
{
    return sDeviceInfoPtr && sDeviceInfoPtr->isValid();
}

DeviceInfo::DeviceInfo()
    : vender()
    , renderer()
    , version()
    , maxTextureSize(0)
    , maxRenderBufferSize(0)
{
}

void DeviceInfo::load()
{
    Global::makeCurrent();
    auto& ggl = Global::functions();

    vender   = std::string((const char*)ggl.glGetString(GL_VENDOR));
    renderer = std::string((const char*)ggl.glGetString(GL_RENDERER));
    version  = std::string((const char*)ggl.glGetString(GL_VERSION));

    ggl.glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    ggl.glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderBufferSize);

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

bool DeviceInfo::isValid() const
{
    return maxTextureSize > 0 && maxRenderBufferSize > 0;
}

} // namespace gl

