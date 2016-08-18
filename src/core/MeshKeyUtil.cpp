#include "core/MeshKeyUtil.h"
#include "core/FFDKey.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
MeshKeyUtil::CreateFace::CreateFace(
        QList<MeshFace*>& aFaceList, MeshEdge& aEdge0, MeshEdge& aEdge1, MeshEdge& aEdge2)
    : mFaceList(aFaceList)
    , mNewFace()
    , mEdges()
{
    mEdges[0] = &aEdge0;
    mEdges[1] = &aEdge1;
    mEdges[2] = &aEdge2;
}

void MeshKeyUtil::CreateFace::exec()
{
    XC_ASSERT(!mNewFace);
    mNewFace.set(new MeshFace());
    redo();
}

void MeshKeyUtil::CreateFace::redo()
{
    mNewFace->set(*mEdges[0], *mEdges[1], *mEdges[2]);
    mFaceList.push_back(mNewFace.get());
    mNewFace.done();
}

void MeshKeyUtil::CreateFace::undo()
{
    XC_ASSERT(!mFaceList.empty());
    XC_ASSERT(mFaceList.back() == mNewFace.get());
    mFaceList.pop_back();
    mNewFace->clear();
    mNewFace.undone();
}

//-------------------------------------------------------------------------------------------------
MeshKeyUtil::CreateEdge::CreateEdge(
        QList<MeshEdge*>& aEdgeList, MeshVtx& aVtx0, MeshVtx& aVtx1)
    : mEdgeList(aEdgeList)
    , mNewEdge()
    , mVtxs()
{
    mVtxs[0] = &aVtx0;
    mVtxs[1] = &aVtx1;
}

void MeshKeyUtil::CreateEdge::exec()
{
    XC_ASSERT(!mNewEdge);
    mNewEdge.set(new MeshEdge());
    redo();
}

void MeshKeyUtil::CreateEdge::redo()
{
    mNewEdge->set(*mVtxs[0], *mVtxs[1]);
    mEdgeList.push_back(mNewEdge.get());
    mNewEdge.done();
}

void MeshKeyUtil::CreateEdge::undo()
{
    XC_ASSERT(!mEdgeList.empty());
    XC_ASSERT(mEdgeList.back() == mNewEdge.get());
    mEdgeList.pop_back();
    mNewEdge->clear();
    mNewEdge.undone();
}

//-------------------------------------------------------------------------------------------------
MeshKeyUtil::CreateVtx::CreateVtx(
        MeshKey& aKey, QList<MeshVtx*>& aVtxList, const QVector2D& aPos)
    : mKey(aKey), mVtxList(aVtxList), mNewVtx(), mPos(aPos)
{
}

void MeshKeyUtil::CreateVtx::exec()
{
    XC_ASSERT(!mNewVtx);
    mNewVtx.set(new MeshVtx(mPos));
    redo();
}

void MeshKeyUtil::CreateVtx::redo()
{
    mNewVtx->setIndex(mVtxList.count());
    mVtxList.push_back(mNewVtx.get());

    auto pos = gl::Vector3::make(mPos.x(), mPos.y(), 0.0f);
    for (auto child : mKey.children())
    {
        XC_ASSERT(child->type() == TimeKeyType_FFD);
        ((FFDKey*)child)->data().pushBackVtx(pos);
    }
    mNewVtx.done();
}

void MeshKeyUtil::CreateVtx::undo()
{
    XC_ASSERT(!mVtxList.empty());
    XC_ASSERT(mVtxList.back() == mNewVtx.get());
    mVtxList.pop_back();

    for (auto child : mKey.children())
    {
        XC_ASSERT(child->type() == TimeKeyType_FFD);
        ((FFDKey*)child)->data().popBackVtx();
    }
    mNewVtx.undone();
}

//-------------------------------------------------------------------------------------------------
MeshKeyUtil::RemoveFace::RemoveFace(QList<MeshFace*>& aFaceList, MeshFace& aDelFace)
    : mFaceList(aFaceList)
    , mDelFace(&aDelFace)
    , mPrevEdges()
    , mIndex()
{
}

void MeshKeyUtil::RemoveFace::exec()
{
    for (int i = 0; i < 3; ++i)
    {
        mPrevEdges[i] = mDelFace->edge(i);
    }
    mIndex = mFaceList.indexOf(mDelFace.get());
    XC_ASSERT(mIndex >= 0);
    redo();
}

void MeshKeyUtil::RemoveFace::redo()
{
    mFaceList.removeAt(mIndex);
    mDelFace->clear();
    mDelFace.done();
}

void MeshKeyUtil::RemoveFace::undo()
{
    mDelFace->set(*mPrevEdges[0], *mPrevEdges[1], *mPrevEdges[2]);
    mFaceList.insert(mIndex, mDelFace.get());
    mDelFace.undone();
}

//-------------------------------------------------------------------------------------------------
MeshKeyUtil::RemoveEdge::RemoveEdge(QList<MeshEdge*>& aEdgeList, MeshEdge& aDelEdge)
    : mEdgeList(aEdgeList)
    , mDelEdge(&aDelEdge)
    , mPrevVtxs()
    , mIndex()
{
}

void MeshKeyUtil::RemoveEdge::exec()
{
    for (int i = 0; i < 2; ++i)
    {
        mPrevVtxs[i] = mDelEdge->vtx(i);
    }
    mIndex = mEdgeList.indexOf(mDelEdge.get());
    XC_ASSERT(mIndex >= 0);
    redo();
}

void MeshKeyUtil::RemoveEdge::redo()
{
    mEdgeList.removeAt(mIndex);
    mDelEdge->clear();
    mDelEdge.done();
}

void MeshKeyUtil::RemoveEdge::undo()
{
    mDelEdge->set(*mPrevVtxs[0], *mPrevVtxs[1]);
    mEdgeList.insert(mIndex, mDelEdge.get());
    mDelEdge.undone();
}

//-------------------------------------------------------------------------------------------------
MeshKeyUtil::RemoveVtx::RemoveVtx(MeshKey& aKey, QList<MeshVtx*>& aVtxList, MeshVtx& aDelVtx)
    : mKey(aKey)
    , mVtxList(aVtxList)
    , mDelVtx(&aDelVtx)
    , mPrevFFDs()
    , mIndex()
{
}

void MeshKeyUtil::RemoveVtx::exec()
{
    mIndex = mVtxList.indexOf(mDelVtx.get());
    XC_ASSERT(mIndex >= 0);
    redo();
}

void MeshKeyUtil::RemoveVtx::redo()
{
    mVtxList.removeAt(mIndex);
    mKey.updateVtxIndices();

    mPrevFFDs.clear();
    for (auto child : mKey.children())
    {
        XC_ASSERT(child->type() == TimeKeyType_FFD);
        mPrevFFDs.push_back(((FFDKey*)child)->data().removeVtx(mIndex));
    }
    mDelVtx.done();
}

void MeshKeyUtil::RemoveVtx::undo()
{
    mVtxList.insert(mIndex, mDelVtx.get());
    mKey.updateVtxIndices();

    int i = 0;
    for (auto child : mKey.children())
    {
        XC_ASSERT(child->type() == TimeKeyType_FFD);
        ((FFDKey*)child)->data().insertVtx(mIndex, mPrevFFDs.at(i));
        ++i;
    }
    mDelVtx.undone();
}

//-------------------------------------------------------------------------------------------------

MeshEdge* MeshKeyUtil::findEdge(const MeshVtx* aVtx0, const MeshVtx* aVtx1)
{
    for (MeshEdgeLinkNode* edgeNode = aVtx0->edges(); edgeNode; edgeNode = edgeNode->next)
    {
        MeshEdge* edge = edgeNode->parent;

        if ((edge->vtx(0) == aVtx0 && edge->vtx(1) == aVtx1) ||
                (edge->vtx(0) == aVtx1 && edge->vtx(1) == aVtx0))
        {
            return edge;
        }
    }
    return nullptr;
}

MeshFace* MeshKeyUtil::findFace(const MeshEdge* aEdge0, const MeshEdge* aEdge1, const MeshEdge* aEdge2)
{
    for (MeshFaceLinkNode* faceNode = aEdge0->faces(); faceNode; faceNode = faceNode->next)
    {
        auto face = faceNode->parent;
        XC_PTR_ASSERT(face);
        auto e0 = face->edge(0);
        auto e1 = face->edge(1);
        auto e2 = face->edge(2);
        XC_PTR_ASSERT(e0);
        XC_PTR_ASSERT(e1);
        XC_PTR_ASSERT(e2);

        if (e0 == aEdge0)
        {
            if ((e1 == aEdge1 && e2 == aEdge2) || (e2 == aEdge1 && e1 == aEdge2))
            {
                return face;
            }
        }
        else if (e1 == aEdge0)
        {
            if ((e0 == aEdge1 && e2 == aEdge2) || (e2 == aEdge1 && e0 == aEdge2))
            {
                return face;
            }
        }
        else if (e2 == aEdge0)
        {
            if ((e0 == aEdge1 && e1 == aEdge2) || (e1 == aEdge1 && e0 == aEdge2))
            {
                return face;
            }
        }
    }
    return nullptr;
}

MeshVtx* MeshKeyUtil::findCommonVtx(const MeshEdge& aEdge0, const MeshEdge& aEdge1)
{
    if (aEdge0.vtx(0) == aEdge1.vtx(0) || aEdge0.vtx(0) == aEdge1.vtx(1))
    {
        return aEdge0.vtx(0);
    }
    else if (aEdge0.vtx(1) == aEdge1.vtx(0) || aEdge0.vtx(1) == aEdge1.vtx(1))
    {
        return aEdge0.vtx(1);
    }
    return nullptr;
}

} // namespace core
