#ifndef GL_DEVICEINFO_H
#define GL_DEVICEINFO_H

#include <QGL>

namespace gl
{

struct DeviceInfo
{
    static const DeviceInfo& instance();
    static void createInstance();

    DeviceInfo();
    bool isValid() const;

    GLint maxTextureSize;
    GLint maxRenderBufferSize;
};

} // namespace gl

#endif // GL_DEVICEINFO_H
