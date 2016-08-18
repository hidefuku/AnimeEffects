#ifndef CTRL_BONE_IMODE
#define CTRL_BONE_IMODE

#include "core/CameraInfo.h"
#include "core/AbstractCursor.h"
#include "core/RenderInfo.h"
#include "ctrl/BoneParam.h"

namespace ctrl {
namespace bone {

class IMode
{
public:
    virtual ~IMode() {}
    virtual void updateParam(const BoneParam&) {}
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&) = 0;
    virtual void renderQt(const core::RenderInfo&, QPainter&) = 0;
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_IMODE

