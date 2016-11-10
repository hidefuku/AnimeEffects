#ifndef CORE_RENDERINFO_H
#define CORE_RENDERINFO_H

#include <QGL>
#include "XC.h"
#include "core/CameraInfo.h"
#include "core/TimeInfo.h"
namespace core { class ClippingFrame; }
namespace core { class DestinationTexturizer; }

namespace core
{

class RenderInfo
{
public:
    RenderInfo()
        : camera()
        , time()
        , framebuffer()
        , dest(0)
        , isGrid(false)
        , nonPosed(false)
        , originMesh(false)
        , clippingId(0)
        , clippingFrame()
        , destTexturizer()
    {
    }

    CameraInfo camera;
    TimeInfo time;
    GLuint framebuffer;
    GLuint dest;
    bool isGrid;
    bool nonPosed;
    bool originMesh;
    uint8 clippingId;
    ClippingFrame* clippingFrame;
    DestinationTexturizer* destTexturizer;
};

} // namespace core

#endif // CORE_RENDERINFO_H
