#include "util/MathUtil.h"
#include "util/CollDetect.h"
#include "cmnd/BasicCommands.h"
#include "cmnd/ScopedMacro.h"
#include "core/Constant.h"
#include "core/TimeLine.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/mesh/mesh_SplitMode.h"
#include "ctrl/mesh/mesh_Renderer.h"
#include "ctrl/mesh/mesh_Notifier.h"

using namespace core;

namespace ctrl {
namespace mesh {

//-------------------------------------------------------------------------------------------------
SplitMode::RelayPoint::RelayPoint()
    : vtx()
    , edge()
    , face()
    , pos()
{
}

//-------------------------------------------------------------------------------------------------
SplitMode::SplitMode(Project& aProject, const Target& aTarget, KeyOwner& aKey)
    : mProject(aProject)
    , mTarget(*aTarget.node)
    , mKeyOwner(aKey)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mFocuser()
    , mMeshAccessor()
    , mBeganSplit(false)
    , mRelayStart()
    , mRelayPoints()
{
    XC_PTR_ASSERT(mKeyOwner.key);
    mMeshAccessor.setKey(*mKeyOwner.key);
    mFocuser.setMesh(mMeshAccessor);
    mFocuser.setTargetMatrix(mTargetMtx);
    mFocuser.setFocusEnable(true, true, true);
}

bool SplitMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    bool updated = false;
    mFocuser.update(aCamera, aCursor);
    auto modelCursorPos = (mTargetInvMtx * QVector3D(aCursor.worldPos())).toVector2D();

    if (!mBeganSplit)
    {
        if (aCursor.isLeftPressState())
        {
            if (mFocuser.vtxFocus())
            {
                mRelayStart = RelayPoint();
                mRelayStart.vtx = mFocuser.vtxFocus();
                mRelayStart.pos = mRelayStart.vtx->vec();
                mRelayEnd = mRelayStart.pos;
                mBeganSplit = true;
            }
            else if (mFocuser.edgeFocus())
            {
                mRelayStart = RelayPoint();
                mRelayStart.edge = mFocuser.edgeFocus();
                mRelayStart.pos = util::CollDetect::getPosOnSegment(
                                      mRelayStart.edge->seg(), modelCursorPos);
                mRelayEnd = mRelayStart.pos;
                mBeganSplit = true;
            }
            else if (mFocuser.faceFocus())
            {
                mRelayStart = RelayPoint();
                mRelayStart.face = mFocuser.faceFocus();
                mRelayStart.pos = modelCursorPos;
                mRelayEnd = mRelayStart.pos;
                mBeganSplit = true;
            }
            mFocuser.clearFocus();
            updated = true;
        }
    }
    else
    {
        updated = true;
        mRelayEnd = modelCursorPos;
        updateRelayPoints(aCamera);

        if (aCursor.isLeftPressState())
        {
            MeshVtx* tail = splitTriangle();
            if (tail)
            {
                mRelayStart.face = nullptr;
                mRelayStart.edge = nullptr;
                mRelayStart.vtx = tail;
                mRelayStart.pos = tail->vec();
                mFocuser.clearFocus();

                updateRelayPoints(aCamera);
            }
        }
        else if (aCursor.isRightPressState())
        {
            mBeganSplit = false;
            mRelayPoints.clear();
            mFocuser.clearFocus();
        }
    }

    return updated || mFocuser.focusChanged();
}

void SplitMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
{
    Renderer renderer(aPainter, aInfo);
    renderer.setAntialiasing(true);
    renderer.setTargetMatrix(mTargetMtx);

    renderer.renderMesh(mMeshAccessor);
    renderer.renderFocus(mFocuser);

    if (mBeganSplit)
    {
        QVector<QVector2D> points;
        if (mRelayStart.face)
        {
            points.push_back(mRelayStart.pos);
        }
        for (auto relay : mRelayPoints)
        {
            points.push_back(relay.pos);
        }
        if (mRelayPoints.size() < 2)
        {
            points.push_back(mRelayEnd);
        }
        renderer.renderSplitter(points);
    }
}

MeshVtx* SplitMode::splitTriangle()
{
    if (mRelayPoints.size() < 2) return nullptr;

    MeshFace* face = mRelayPoints[1].face;
    MeshEdge* edge[2] = {};
    QVector2D posOnEdge[2] = {};

    if (mRelayPoints[0].edge && mRelayPoints[1].vtx)
    {
        edge[0] = mRelayPoints[0].edge;
        posOnEdge[0] = mRelayPoints[0].pos;
    }
    else if (mRelayPoints[0].vtx && mRelayPoints[1].edge)
    {
        edge[0] = mRelayPoints[1].edge;
        posOnEdge[0] = mRelayPoints[1].pos;
    }
    else if (mRelayPoints[0].edge && mRelayPoints[1].edge)
    {
        for (int i = 0; i < 2; ++i)
        {
            edge[i] = mRelayPoints[i].edge;
            posOnEdge[i] = mRelayPoints[i].pos;
        }
    }
    else
    {
        return nullptr;
    }
    XC_PTR_ASSERT(face);
    XC_PTR_ASSERT(edge[0]);

    cmnd::Stack& stack = mProject.commandStack();
    auto eventType = TimeLineEvent::Type_ChangeKeyValue;

    MeshVtx* tailVtx = nullptr;
    {
        cmnd::ScopedMacro macro(stack, "split triangle");
        // set notifier
        macro.grabListener(new Notifier(mProject, mTarget, *mKeyOwner.key, eventType));
        // push command
        if (edge[1])
        {
            stack.push(mMeshAccessor.createSplitter(
                           *face, *edge[0], posOnEdge[0],
                           *edge[1], posOnEdge[1], &tailVtx));
        }
        else
        {
            stack.push(mMeshAccessor.createSplitter(
                           *face, *edge[0], posOnEdge[0], &tailVtx));
        }
    }
    ///@attention "tailVtx" was assigned when the command executed.

    if (!edge[1] && mRelayPoints[1].vtx)
    {
        return mRelayPoints[1].vtx;
    }
    else
    {
        return tailVtx;
    }
}

QVector2D SplitMode::getScreenPos(
        const core::CameraInfo& aCamera, const QVector2D& aModelPos) const
{
    return aCamera.toScreenPos(mTargetMtx * QVector3D(aModelPos)).toVector2D();
}

QVector2D SplitMode::getModelPos(
        const core::CameraInfo& aCamera, const QVector2D& aScreenPos) const
{
    auto worldPos = QVector3D(aCamera.toWorldPos(aScreenPos));
    return (mTargetInvMtx * worldPos).toVector2D();
}

util::Segment2D SplitMode::getScreenSeg(
        const core::CameraInfo& aCamera, const util::Segment2D& aSeg) const
{
    const QVector2D s = getScreenPos(aCamera, aSeg.start);
    const QVector2D e = getScreenPos(aCamera, aSeg.end());
    return util::Segment2D(s, e - s);
}

SplitMode::RelayPoint SplitMode::getIntersection(
        const CameraInfo& aCamera, const MeshFace& aFace,
        const util::Segment2D& aSeg, const MeshEdge* aIgnoreEdge,
        const MeshVtx* aIgnoreVtx)
{
    static const float kVtxSqRadius = 8.0f * 8.0f;

    RelayPoint result;
    const util::Segment2D seg = getScreenSeg(aCamera, aSeg);

    auto vertices = aFace.vertices();

    for (int i = 0; i < 3; ++i)
    {
        XC_PTR_ASSERT(vertices[i]);
        if (vertices[i] == aIgnoreVtx) continue;

        auto vtxPos = getScreenPos(aCamera, vertices[i]->vec());
        if (util::CollDetect::getMinDistanceSquared(seg, vtxPos) <= kVtxSqRadius)
        {
            result.vtx = vertices[i];
            result.pos = vertices[i]->vec();
            return result;
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        auto edge = aFace.edge(i);
        if (edge == aIgnoreEdge) continue;
        if (edge->vtx(0) == aIgnoreVtx || edge->vtx(1) == aIgnoreVtx) continue;

        auto edgeSeg = getScreenSeg(aCamera, edge->seg());
        auto intersect = util::CollDetect::getIntersection(seg, edgeSeg);
        if (intersect.first)
        {
            result.edge = edge;
            result.pos = getModelPos(aCamera, intersect.second);
        }
    }

    return result;
}

void SplitMode::updateRelayPoints(const core::CameraInfo& aCamera)
{
    XC_ASSERT(mBeganSplit);
    util::Segment2D cursorSeg(mRelayStart.pos, mRelayEnd - mRelayStart.pos);

    mRelayPoints.clear();

    if (mRelayStart.face)
    {
        RelayPoint start = getIntersection(aCamera, *mRelayStart.face, cursorSeg);
        if (start.vtx || start.edge)
        {
            mRelayPoints.push_back(start);
        }
        cursorSeg = util::Segment2D(start.pos, mRelayEnd - start.pos);
    }
    else
    {
        mRelayPoints.push_back(mRelayStart);
    }

    if (mRelayPoints.empty())
    {
        return;
    }

    if (mRelayPoints[0].edge)
    {
        auto startEdge = mRelayPoints[0].edge;

        for (MeshFaceLinkNode* it = startEdge->faces(); it; it = it->next)
        {
            auto face = it->parent;
            XC_PTR_ASSERT(face);

            RelayPoint res = getIntersection(aCamera, *face, cursorSeg, startEdge);

            if (res.vtx && res.vtx != face->oppositeVtx(*startEdge))
            {
                continue;
            }

            if (res.vtx || res.edge)
            {
                res.face = face;
                mRelayPoints.push_back(res);
                break;
            }
        }
    }
    else if (mRelayPoints[0].vtx)
    {
        QVector<MeshFace*> checkFaces;

        auto startVtx = mRelayPoints[0].vtx;
        for (MeshEdgeLinkNode* it = startVtx->edges(); it; it = it->next)
        {
            auto edge = it->parent;
            XC_PTR_ASSERT(edge);
            for (MeshFaceLinkNode* faceIt = edge->faces(); faceIt; faceIt = faceIt->next)
            {
                auto face = faceIt->parent;
                XC_PTR_ASSERT(face);
                if (!checkFaces.contains(face))
                {
                    checkFaces.push_back(face);
                }
            }
        }

        for (auto face : checkFaces)
        {
            RelayPoint res = getIntersection(
                                 aCamera, *face, cursorSeg, nullptr, startVtx);
            if (res.edge && res.edge == face->oppositeEdge(*startVtx))
            {
                res.face = face;
                mRelayPoints.push_back(res);
                break;
            }
        }
    }
}

} // namespace mesh
} // namespace ctrl
