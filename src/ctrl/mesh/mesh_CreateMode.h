#ifndef CTRL_MESH_CREATEMODE_H
#define CTRL_MESH_CREATEMODE_H

#include "core/Project.h"
#include "core/ObjectNode.h"
#include "ctrl/mesh/mesh_IMode.h"
#include "ctrl/mesh/mesh_KeyOwner.h"
#include "ctrl/mesh/mesh_Target.h"
#include "ctrl/mesh/mesh_Focuser.h"
#include "ctrl/mesh/mesh_MeshAccessor.h"
#include "ctrl/mesh/mesh_VtxMover.h"

namespace ctrl {
namespace mesh {

//-------------------------------------------------------------------------------------------------
class CreateMode : public IMode
{
public:
    CreateMode(core::Project& aProject, const Target& aTarget, KeyOwner& aKey);
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&);
    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    enum State
    {
        State_Idle,
        State_Move,  ///< move vertex
        State_New,   ///< make new triangle
        State_Add,   ///< extrude triangle from edge
        State_TERM
    };

    QVector2D getModelPos(const core::AbstractCursor&);
    MeshFace* pushTriangle(
            const QVector<QVector2D>& aPos,
            const QVector<MeshVtx*>& aRef);
    void moveVtx(MeshVtx& aVtx, const QVector2D& aPos);

    void initIdle();
    void initMove();
    void initNew(const QVector2D& aModelPos);
    void initAdd();

    bool procIdle(const core::AbstractCursor&);
    void procMove(const core::AbstractCursor&);
    void procNew(const core::AbstractCursor&);
    void procAdd(const core::AbstractCursor&);

    core::Project& mProject;
    core::ObjectNode& mTarget;
    KeyOwner& mKeyOwner;
    QMatrix4x4 mTargetMtx;
    QMatrix4x4 mTargetInvMtx;
    Focuser mFocuser;
    MeshAccessor mMeshAccessor;

    State mState;
    QVector<QVector2D> mDanglingPos;
    QVector<MeshVtx*> mDanglingRef;
    QVector2D mCursorPos;
    Focuser::Focus mLastFocus;
    VtxMover* mMoverRef;
};

} // namespace mesh
} // namespace ctrl

#endif // CTRL_MESH_CREATEMODE_H
