#include <QPolygonF>
#include "util/CollDetect.h"
#include "ctrl/mesh/mesh_Focuser.h"

namespace
{
static const float kVtxSqRadius = 8.0f * 8.0f;
static const float kEdgeSqRadius = 8.0f * 8.0f;
}

namespace ctrl {
namespace mesh {

Focuser::Focuser()
    : mMesh()
    , mTargetMtx()
    , mFocus()
    , mFocusChanged()
    , mSelecting()
{
    setFocusEnable();
}

void Focuser::setMesh(MeshAccessor& aMesh)
{
    mMesh = &aMesh;
}

void Focuser::setTargetMatrix(const QMatrix4x4& aMtx)
{
    mTargetMtx = aMtx;
}

void Focuser::update(const core::CameraInfo& aCamera, const core::AbstractCursor& aCursor)
{
    auto prev = mFocus;
    clearFocus();

    if (!mMesh) return;

    const QVector2D focusPos = aCursor.screenPos();

    // vertex
    if (mEnable[0])
    {
        for (auto& vtx : mMesh->vertices())
        {
            const QVector2D vtxPos = getScreenPos(aCamera, vtx->vec());

            if ((focusPos - vtxPos).lengthSquared() < kVtxSqRadius)
            {
                mFocus.vtx = vtx;
                updateFocusChanged(prev, mFocus);
                return;
            }
        }
    }

    // edge
    if (mEnable[1])
    {
        for (auto& edge : mMesh->edges())
        {
            const QVector2D v0 = getScreenPos(aCamera, edge->vtx(0)->vec());
            const QVector2D v1 = getScreenPos(aCamera, edge->vtx(1)->vec());
            auto c = util::CollDetect::getPosOnSegment(
                        util::Segment2D(v0, v1 - v0), focusPos);
            if ((focusPos - c).lengthSquared() < kEdgeSqRadius)
            {
                mFocus.edge = edge;
                updateFocusChanged(prev, mFocus);
                return;
            }
        }
    }

    // face
    if (mEnable[2])
    {
        for (auto& face : mMesh->faces())
        {
            auto vtx = face->vertices();
            QPolygonF poly;
            poly.push_back(getScreenPos(aCamera, vtx[0]->vec()).toPointF());
            poly.push_back(getScreenPos(aCamera, vtx[1]->vec()).toPointF());
            poly.push_back(getScreenPos(aCamera, vtx[2]->vec()).toPointF());

            if (poly.containsPoint(focusPos.toPointF(), Qt::OddEvenFill))
            {
                mFocus.face = face;
                updateFocusChanged(prev, mFocus);
                return;
            }
        }
    }

    updateFocusChanged(prev, mFocus);
}

void Focuser::updateFocusChanged(const Focus& aPrev, const Focus& aNext)
{
    mFocusChanged =
            (aPrev.vtx != aNext.vtx) ||
            (aPrev.edge != aNext.edge) ||
            (aPrev.face != aNext.face);
}

QVector2D Focuser::getScreenPos(
        const core::CameraInfo& aCamera, const QVector2D& aModelPos) const
{
    return aCamera.toScreenPos(mTargetMtx * QVector3D(aModelPos)).toVector2D();
}

void Focuser::setFocusEnable(bool aVtx, bool aEdge, bool aFace)
{
    mEnable[0] = aVtx;
    mEnable[1] = aEdge;
    mEnable[2] = aFace;
}

} // namespace mesh
} // namespace ctrl

