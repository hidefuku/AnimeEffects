#ifndef CTRL_MESH_VTXMOVER
#define CTRL_MESH_VTXMOVER

#include "cmnd/Stable.h"
#include "core/MeshKey.h"

namespace ctrl {
namespace mesh {

class VtxMover : public cmnd::Stable
{
    core::MeshKey& mKey;
    core::MeshVtx& mVtx;
    QVector2D mPrev;
    QVector2D mNext;
    bool mDone;

public:
    VtxMover(core::MeshKey& aKey, core::MeshVtx& aVtx, const QVector2D& aPos)
        : mKey(aKey), mVtx(aVtx), mPrev(), mNext(aPos), mDone(false) {}

    const core::MeshVtx* currentVtx() const
    {
        return &mVtx;
    }

    void modify(const QVector2D& aPos)
    {
        mNext = aPos;
        if (mDone) mKey.moveVtx(mVtx, mNext);
    }

    virtual void undo()
    {
        mKey.moveVtx(mVtx, mPrev);
        mDone = false;
    }

    virtual void redo()
    {
        mPrev = mVtx.vec();
        mKey.moveVtx(mVtx, mNext);
        mDone = true;
    }
};

} // namespace mesh
} // namespace ctrl

#endif // CTRL_MESH_VTXMOVER

