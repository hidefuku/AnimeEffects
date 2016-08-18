#ifndef CTRL_SRT_IMODE
#define CTRL_SRT_IMODE

#include "core/CameraInfo.h"
#include "core/AbstractCursor.h"
#include "core/RenderInfo.h"
#include "ctrl/SRTParam.h"

namespace ctrl {
namespace srt {

class IMode
{
public:
    virtual ~IMode() {}
    virtual void updateParam(const SRTParam&) {}
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&) = 0;
    virtual void renderQt(const core::RenderInfo&, QPainter&) = 0;
};

} // namespace srt
} // namespace ctrl

#endif // CTRL_SRT_IMODE

