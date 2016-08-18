#ifndef CTRL_MESH_DELETEMODE_H
#define CTRL_MESH_DELETEMODE_H

#include "core/Project.h"
#include "core/ObjectNode.h"
#include "ctrl/mesh/mesh_IMode.h"
#include "ctrl/mesh/mesh_KeyOwner.h"
#include "ctrl/mesh/mesh_Target.h"
#include "ctrl/mesh/mesh_Focuser.h"
#include "ctrl/mesh/mesh_MeshAccessor.h"

namespace ctrl {
namespace mesh {

//-------------------------------------------------------------------------------------------------
class DeleteMode : public IMode
{
public:
    DeleteMode(core::Project& aProject, const Target& aTarget, KeyOwner& aKey);
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&);
    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    QVector2D getModelPos(const core::AbstractCursor&);
    void removeVtx(MeshVtx& aVtx);
    void removeFace(MeshFace& aFace);

    core::Project& mProject;
    core::ObjectNode& mTarget;
    KeyOwner& mKeyOwner;
    QMatrix4x4 mTargetMtx;
    QMatrix4x4 mTargetInvMtx;
    Focuser mFocuser;
    MeshAccessor mMeshAccessor;
};

} // namespace mesh
} // namespace ctrl

#endif // CTRL_MESH_DELETEMODE_H
