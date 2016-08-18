#ifndef CTRL_MESH_IMODE
#define CTRL_MESH_IMODE

#include "core/CameraInfo.h"
#include "core/AbstractCursor.h"
#include "core/RenderInfo.h"
#include "ctrl/MeshParam.h"

namespace ctrl {
namespace mesh {

class IMode
{
public:
    virtual ~IMode() {}
    virtual void updateParam(const MeshParam&) {}
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&) = 0;
    virtual void renderQt(const core::RenderInfo&, QPainter&) = 0;
};

} // namespace mesh
} // namespace ctrl

#endif // CTRL_MESH_IMODE
