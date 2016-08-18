#ifndef CTRL_MESH_SPLITMODE_H
#define CTRL_MESH_SPLITMODE_H

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
class SplitMode : public IMode
{
public:
    struct RelayPoint
    {
        RelayPoint();
        core::MeshVtx* vtx;
        core::MeshEdge* edge;
        core::MeshFace* face;
        QVector2D pos;
    };

    SplitMode(core::Project& aProject, const Target& aTarget, KeyOwner& aKey);
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&);
    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    MeshVtx* splitTriangle();
    QVector2D getScreenPos(
            const core::CameraInfo&, const QVector2D& aModelPos) const;
    QVector2D getModelPos(
            const core::CameraInfo&, const QVector2D& aScreenPos) const;
    util::Segment2D getScreenSeg(
            const core::CameraInfo& aCamera, const util::Segment2D& aSeg) const;

    RelayPoint getIntersection(
            const core::CameraInfo& aCamera,
            const core::MeshFace& aFace,
            const util::Segment2D& aSeg,
            const core::MeshEdge* aIgnoreEdge = nullptr,
            const core::MeshVtx* aIgnoreVtx = nullptr);
    void updateRelayPoints(const core::CameraInfo&);

    core::Project& mProject;
    core::ObjectNode& mTarget;
    KeyOwner& mKeyOwner;
    QMatrix4x4 mTargetMtx;
    QMatrix4x4 mTargetInvMtx;
    Focuser mFocuser;
    MeshAccessor mMeshAccessor;

    bool mBeganSplit;
    RelayPoint mRelayStart;
    QVector2D mRelayEnd;
    QVector<RelayPoint> mRelayPoints;
};

} // namespace mesh
} // namespace ctrl

#endif // CTRL_MESH_SPLITMODE_H
