#ifndef CTRL_FFD_IMODE_H
#define CTRL_FFD_IMODE_H

#include "core/CameraInfo.h"
#include "core/AbstractCursor.h"
#include "core/RenderInfo.h"
#include "ctrl/FFDParam.h"

namespace ctrl {
namespace ffd {

class IMode
{
public:
    virtual ~IMode() {}
    virtual void updateParam(const FFDParam&) {}
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&) = 0;
    virtual void renderQt(const core::RenderInfo&, QPainter&) = 0;
};

} // namespace ffd
} // namespace ctrl


#endif // CTRL_FFD_IMODE_H
