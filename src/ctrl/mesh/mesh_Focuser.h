#ifndef CTRL_MESH_FOCUSER_H
#define CTRL_MESH_FOCUSER_H

#include "core/AbstractCursor.h"
#include "core/CameraInfo.h"
#include "ctrl/mesh/mesh_MeshAccessor.h"

namespace ctrl {
namespace mesh {

class Focuser
{
public:
    struct Focus
    {
        Focus()
            : vtx()
            , edge()
            , face()
        {}
        MeshVtx* vtx;
        MeshEdge* edge;
        MeshFace* face;
    };

    Focuser();
    void setMesh(MeshAccessor& aMesh);
    void setTargetMatrix(const QMatrix4x4& aMtx);

    void update(const core::CameraInfo&, const core::AbstractCursor&);

    void setFocusEnable(bool aVtx = true, bool aEdge = true, bool aFace = true);
    void clearFocus() { mFocus = Focus(); }
    bool focusChanged() const { return mFocusChanged; }
    Focus focus() const { return mFocus; }
    MeshVtx* vtxFocus() const { return mFocus.vtx; }
    MeshEdge* edgeFocus() const { return mFocus.edge; }
    MeshFace* faceFocus() const { return mFocus.face; }

    void selectFace(MeshFace* aFace) { mSelecting.face = aFace; }
    void selectEdge(MeshEdge* aEdge) { mSelecting.edge = aEdge; }
    void selectVtx(MeshVtx* aVtx) { mSelecting.vtx = aVtx; }
    MeshFace* selectingFace() const { return mSelecting.face; }
    MeshEdge* selectingEdge() const { return mSelecting.edge; }
    MeshVtx* selectingVtx() const { return mSelecting.vtx; }
    void clearSelection() { mSelecting = Focus(); }

private:
    QVector2D getScreenPos(const core::CameraInfo&, const QVector2D& aModelPos) const;
    void updateFocusChanged(const Focus& aPrev, const Focus& aNext);

    MeshAccessor* mMesh;
    QMatrix4x4 mTargetMtx;
    Focus mFocus;
    bool mFocusChanged;
    Focus mSelecting;
    bool mEnable[3];
};

} // namespace mesh
} // namespace ctrl

#endif // CTRL_MESH_FOCUSER_H
