#ifndef CTRL_POSE_IMODE_H
#define CTRL_POSE_IMODE_H

#include "core/CameraInfo.h"
#include "core/AbstractCursor.h"
#include "core/RenderInfo.h"
#include "ctrl/PoseParam.h"

namespace ctrl {
namespace pose {

class IMode
{
public:
    virtual ~IMode() {}
    virtual void updateParam(const PoseParam&) {}
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&) = 0;
    virtual void renderQt(const core::RenderInfo&, QPainter&) = 0;
};

} // namespace pose
} // namespace ctrl

#endif // CTRL_POSE_IMODE_H
