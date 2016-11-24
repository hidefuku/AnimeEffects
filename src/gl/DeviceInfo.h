#ifndef GL_DEVICEINFO_H
#define GL_DEVICEINFO_H

#include <string>
#include <QGL>

namespace gl
{

struct DeviceInfo
{
    static void setInstance(const DeviceInfo* aInstance);
    static bool validInstanceExists();
    static const DeviceInfo& instance();

    DeviceInfo();
    void load();
    bool isValid() const;

    std::string vender;
    std::string renderer;
    std::string version;
    GLint maxTextureSize;
    GLint maxRenderBufferSize;
};

} // namespace gl

#endif // GL_DEVICEINFO_H
