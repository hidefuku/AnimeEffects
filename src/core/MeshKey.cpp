#include <float.h>
#include <QPolygonF>
#include "XC.h"
#include "util/CollDetect.h"
#include "util/MathUtil.h"
#include "cmnd/BasicCommands.h"
#include "cmnd/Scalable.h"
#include "core/Constant.h"
#include "core/MeshKey.h"
#include "core/MeshKeyUtil.h"
#include "core/FFDKey.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
MeshVtx::MeshVtx()
    : x(0.0f)
    , y(0.0f)
    , mEdges()
    , mIndex(-1)
{
}

MeshVtx::MeshVtx(float aX, float aY)
    : x(aX)
    , y(aY)
    , mEdges()
{
}

MeshVtx::MeshVtx(const QVector2D& aPos)
    : x(aPos.x())
    , y(aPos.y())
    , mEdges()
    , mIndex(-1)
{
}

MeshVtx::~MeshVtx()
{
    XC_ASSERT(!hasParent());
}

void MeshVtx::link(MeshEdgeLinkNode& aEdge)
{
    MeshEdgeLink current = mEdges;
    aEdge.child = this;
    aEdge.prev = nullptr;
    aEdge.next = current;
    if (current) current->prev = &aEdge;

    mEdges = &aEdge;

}

void MeshVtx::unlink(MeshEdgeLinkNode& aEdge)
{
    MeshEdgeLink edge = &aEdge;
    MeshEdgeLink prev = edge->prev;
    MeshEdgeLink next = edge->next;
    if (prev) prev->next = next;
    if (next) next->prev = prev;
    edge->child = nullptr;
    edge->prev = nullptr;
    edge->next = nullptr;

    if (&aEdge == mEdges)
    {
        mEdges = next;
    }
}

bool MeshVtx::hasOtherParents(const QVector<MeshEdge*>& aEdges) const
{
    // each edge
    for (MeshEdgeLinkNode* edgeNode = mEdges; edgeNode; edgeNode = edgeNode->next)
    {
        MeshEdge* edge = edgeNode->parent;
        if (!aEdges.contains(edge))
        {
            return true;
        }
    }
    return false;
}

int MeshVtx::connectionCount() const
{
    int count = 0;
    for (MeshEdgeLinkNode* edgeNode = mEdges; edgeNode; edgeNode = edgeNode->next)
    {
        ++count;
    }
    return count;
}

//-------------------------------------------------------------------------------------------------
MeshEdge::MeshEdge()
    : mFaces()
{
    for (int i = 0; i < 2; ++i)
    {
        mLink[i].parent = this;
    }
}

MeshEdge::~MeshEdge()
{
    XC_ASSERT(!hasParent());
    clear();
}

void MeshEdge::set(MeshVtx& aVp0, MeshVtx& aVp1)
{
    clear();
    aVp0.link(mLink[0]);
    aVp1.link(mLink[1]);
}

void MeshEdge::clear()
{
    for (int i = 0; i < 2; ++i)
    {
        if (mLink[i].child)
        {
            mLink[i].child->unlink(mLink[i]);
        }
    }
}

bool MeshEdge::isEquals(const MeshVtx& aVp0, const MeshVtx& aVp1) const
{
    return ((&aVp0 == mLink[0].child && &aVp1 == mLink[1].child) ||
            (&aVp0 == mLink[1].child && &aVp1 == mLink[0].child));
}

bool MeshEdge::hasMultiParents() const
{
    return mFaces && mFaces->next;
}

bool MeshEdge::hasOtherParents(const QVector<MeshFace*>& aFaces) const
{
    // each face
    for (MeshFaceLinkNode* faceNode = faces(); faceNode; faceNode = faceNode->next)
    {
        if (!aFaces.contains(faceNode->parent))
        {
            return true;
        }
    }
    return false;
}

void MeshEdge::link(MeshFaceLinkNode& aFace)
{
    MeshFaceLink current = mFaces;
    aFace.child = this;
    aFace.prev = nullptr;
    aFace.next = current;
    if (current) current->prev = &aFace;

    mFaces = &aFace;
}

void MeshEdge::unlink(MeshFaceLinkNode& aFace)
{
    MeshFaceLink face = &aFace;
    MeshFaceLink prev = face->prev;
    MeshFaceLink next = face->next;
    if (prev) prev->next = next;
    if (next) next->prev = prev;
    face->child = nullptr;
    face->prev = nullptr;
    face->next = nullptr;

    if (&aFace == mFaces)
    {
        mFaces = next;
    }
}

void MeshEdge::rawInit(MeshVtx &aVp0, MeshVtx &aVp1)
{
    aVp0.link(mLink[0]);
    aVp1.link(mLink[1]);
}

//-------------------------------------------------------------------------------------------------
MeshFace::MeshFace()
{
    mClockwise = false;
    for (int i = 0; i < 3; ++i)
    {
        mLink[i].parent = this;
        mInverse[i] = false;

    }
}

MeshFace::~MeshFace()
{
    clear();
}

bool MeshFace::has(const MeshEdge& aEdge) const
{
    for (int i = 0; i < 3; ++i)
    {
        if (mLink[i].child == &aEdge) return true;
    }
    return false;
}

bool MeshFace::has(const MeshVtx& aVtx) const
{
    auto vtxs = vertices();
    for (int i = 0; i < 3; ++i)
    {
        if (vtxs[i] == &aVtx) return true;
    }
    return false;
}

bool MeshFace::hasChildren() const
{
    return mLink[0].child || mLink[1].child || mLink[2].child;
}

void MeshFace::set(MeshEdge& aEp0, MeshEdge& aEp1, MeshEdge& aEp2)
{
    clear();
    aEp0.link(mLink[0]);
    aEp1.link(mLink[1]);
    aEp2.link(mLink[2]);
    updateDir();
}

void MeshFace::clear()
{
    for (int i = 0; i < 3; ++i)
    {
        if (mLink[i].child)
        {
            mLink[i].child->unlink(mLink[i]);
        }
    }
}

void MeshFace::updateDir()
{
    for (int i = 0; i < 3; ++i)
    {
        if (!mLink[i].child) return;
    }

    std::array<MeshVtx*, 3> vtx;
    vtx[0] = mLink[0].child->vtx(0);
    vtx[1] = mLink[0].child->vtx(1);
    vtx[2] = mLink[1].child->vtx(0);
    if (vtx[0] == vtx[2])
    {
        vtx[2] = mLink[1].child->vtx(1);
        mInverse[0] = true;
        mInverse[1] = false;
    }
    else if (vtx[1] == vtx[2])
    {
        vtx[2] = mLink[1].child->vtx(1);
        mInverse[0] = false;
        mInverse[1] = false;
    }
    else
    {
        mInverse[0] = false;
        mInverse[1] = true;
    }
    mInverse[2] = (vtx[2] != mLink[2].child->vtx(0));

    auto seg01 = util::Segment2D(vtx[0]->vec(), vtx[1]->vec() - vtx[0]->vec());
    mClockwise = (util::CollDetect::getPosSide(seg01, vtx[2]->vec()) == 1);
}

MeshEdge* MeshFace::findEdge(const MeshVtx& aVtx0, const MeshVtx& aVtx1) const
{
    for (int i = 0; i < 3; ++i)
    {
        if (mLink[i].child->isEquals(aVtx0, aVtx1))
        {
            return mLink[i].child;
        }
    }
    return nullptr;
}

std::array<MeshVtx*, 3> MeshFace::vertices() const
{
    XC_PTR_ASSERT(mLink[0].child);
    XC_PTR_ASSERT(mLink[1].child);
    XC_PTR_ASSERT(mLink[2].child);

    std::array<MeshVtx*, 3> vtx;
    vtx[0] = mLink[0].child->vtx(0);
    vtx[1] = mLink[0].child->vtx(1);
    vtx[2] = mLink[1].child->vtx(0);
    if (vtx[2] == vtx[0] || vtx[2] == vtx[1])
    {
        vtx[2] = mLink[1].child->vtx(1);
    }
    return vtx;
}

QVector2D MeshFace::vnorm(int aIndex) const
{
    QVector2D dir = mLink[aIndex].child->dir();
    if (dir.length() < core::Constant::normalizable())
    {
        return QVector2D(1.0f, 0.0f);
    }
    dir.normalize();
    dir = util::MathUtil::getRotateVector90Deg(dir);

    if (mClockwise)
    {
        return mInverse[aIndex] ? dir : -dir;
    }
    else
    {
        return mInverse[aIndex] ? -dir : dir;
    }
}

bool MeshFace::isClockwise() const
{
    return mClockwise;
}

bool MeshFace::containsPoint(const QVector2D& aPoint) const
{
    auto vtx = vertices();
    QPolygonF poly;
    poly.push_back(vtx[0]->vec().toPointF());
    poly.push_back(vtx[1]->vec().toPointF());
    poly.push_back(vtx[2]->vec().toPointF());
    return poly.containsPoint(aPoint.toPointF(), Qt::OddEvenFill);
}

MeshEdge* MeshFace::nearestEdge(const QVector2D& aPoint) const
{
    if (containsPoint(aPoint))
    {
        MeshEdge* edge = mLink[0].child;
        float minLength = FLT_MAX;
        for (int i = 0; i < 3; ++i)
        {
            auto seg = mLink[i].child->seg();
            float length = util::CollDetect::getMinDistanceSquared(seg, aPoint);
            if (length < minLength)
            {
                edge = mLink[i].child;
                minLength = length;
            }
        }
        return edge;
    }
    else
    {
        auto vtx = vertices();
        auto center = (vtx[0]->vec() + vtx[1]->vec() + vtx[2]->vec()) / 3.0f;
        auto vec = aPoint - center;

        MeshEdge* edge = mLink[0].child;
        float maxDot = FLT_MIN;

        for (int i = 0; i < 3; ++i)
        {
            auto v = vnorm(i);
            float dot = QVector2D::dotProduct(v, vec);
            if (dot > maxDot)
            {
                edge = mLink[i].child;
                maxDot = dot;
            }
        }
        return edge;
    }
}

MeshVtx* MeshFace::oppositeVtx(const MeshEdge& aEdge) const
{
    for (int i = 0; i < 3; ++i)
    {
        MeshEdge* a = mLink[i].child;
        if (a == &aEdge)
        {
            MeshEdge* b = mLink[(i + 1) % 3].child;
            return a->has(*(b->vtx(0))) ? b->vtx(1) : b->vtx(0);
        }
    }
    return nullptr;
}

MeshEdge* MeshFace::oppositeEdge(const MeshVtx& aVtx) const
{
    for (int i = 0; i < 3; ++i)
    {
        MeshEdge* a = mLink[i].child;
        if (a->vtx(0) != &aVtx && a->vtx(1) != &aVtx) return a;
    }
    return nullptr;
}

void MeshFace::rawInit(MeshEdge& aEp0, MeshEdge& aEp1, MeshEdge& aEp2)
{
    aEp0.link(mLink[0]);
    aEp1.link(mLink[1]);
    aEp2.link(mLink[2]);
    updateDir();
}

//-------------------------------------------------------------------------------------------------
MeshKey::Data::Data()
    : mVertices()
    , mEdges()
    , mFaces()
    , mPositions()
    , mTexCoords()
    , mIndices()
    , mMeshBuffer()
    , mOwner()
{
}

MeshKey::Data::~Data()
{
    destroy();
}

void MeshKey::Data::destroy()
{
    // kill from parents
    qDeleteAll(mFaces);
    qDeleteAll(mEdges);
    qDeleteAll(mVertices);

    mFaces.clear();
    mEdges.clear();
    mVertices.clear();
}

LayerMesh::MeshBuffer& MeshKey::Data::getMeshBuffer()
{
    if (mMeshBuffer.isNull())
    {
        mMeshBuffer.reset(new MeshBuffer());
    }
    mMeshBuffer->reserve(vertexCount());
    return *mMeshBuffer;
}

void MeshKey::Data::resetArrayedConnection(
        ArrayedConnectionList& aDest,
        const gl::Vector3* aPositions) const
{
    aDest.resetIndexRanges(vertexCount());
    aDest.clearBlocks();

    ArrayedConnection* curBlock = aDest.pushNewBlock();
    curBlock->vertexRange.setMin(0);

    int i = 0;
    for (auto vtx : mVertices)
    {
        const int connectCount = vtx->connectionCount();

        // setup positions
        if (curBlock->positionCount >= ArrayedConnection::kMaxCount - connectCount)
        {
            curBlock->vertexRange.setMax(i - 1);

            curBlock = aDest.pushNewBlock();
            curBlock->vertexRange.setMin(i);
        }

        const int posBegin = curBlock->positionCount;

        for (auto edgeNode = vtx->edges(); edgeNode; edgeNode = edgeNode->next)
        {
            auto edge = edgeNode->parent;
            auto vtx2 = edge->vtx(0) != vtx ? edge->vtx(0) : edge->vtx(1);
            auto connectIndex = vtx2->index();
            XC_ASSERT(0 <= connectIndex);
            auto offset = aPositions[connectIndex] -
                    gl::Vector3::make(vtx2->x, vtx2->y, 0.0f);
            curBlock->pushPosition(offset.vec2());
        }

        aDest.indexRanges[i].set(posBegin, connectCount);
        ++i;
    }
    if (i > 0)
    {
        curBlock->vertexRange.setMax(i - 1);
    }
    aDest.destroyUnuseBlocks();
}

//-------------------------------------------------------------------------------------------------
MeshKey::MeshKey()
    : mData()
{
    mData.mOwner = this;
}

bool MeshKey::serialize(Serializer& aOut) const
{
    // image size
    //aOut.write(mData.mSize);

    // vertex count
    aOut.write((int)mData.mVertices.count());

    // vertices
    for (auto vtx : mData.mVertices)
    {
        aOut.write(vtx->vec());
    }

    // edge count
    aOut.write((int)mData.mEdges.count());

    // edges
    for (auto edge : mData.mEdges)
    {
        aOut.write((int)edge->vtx(0)->index());
        aOut.write((int)edge->vtx(1)->index());
    }

    // face count
    aOut.write((int)mData.mFaces.count());

    // faces
    for (auto face : mData.mFaces)
    {
        aOut.write((int)mData.mEdges.indexOf(face->edge(0)));
        aOut.write((int)mData.mEdges.indexOf(face->edge(1)));
        aOut.write((int)mData.mEdges.indexOf(face->edge(2)));
    }

    return aOut.checkStream();
}

bool MeshKey::deserialize(Deserializer& aIn)
{
    aIn.pushLogScope("MeshKey");

    mData.destroy();

    // image size
    //aIn.read(mData.mSize);

    // vertex count
    int vtxCount = 0;
    aIn.read(vtxCount);
    if (vtxCount < 0)
        return aIn.errored("invalid vertex count");

    // vertices
    for (int i = 0; i < vtxCount; ++i)
    {
        QVector2D vec;
        aIn.read(vec);
        auto vtx = new MeshVtx(vec);
        vtx->setIndex(i);
        mData.mVertices.push_back(vtx);
    }
    // check failure
    if (aIn.failure())
        return aIn.errored("stream error");

    // edge count
    int edgeCount = 0;
    aIn.read(edgeCount);
    if (edgeCount < 0)
        return aIn.errored("invalid edge count");

    // edges
    for (int i = 0; i < edgeCount; ++i)
    {
        int vtxIdx[2] = {};
        aIn.read(vtxIdx[0]);
        aIn.read(vtxIdx[1]);

        if (!xc_contains(vtxIdx[0], 0, vtxCount - 1) ||
            !xc_contains(vtxIdx[1], 0, vtxCount - 1))
        {
            return aIn.errored("invalid edge reference");
        }

        auto edge = new MeshEdge();
        edge->rawInit(
                *mData.mVertices.at(vtxIdx[0]),
                *mData.mVertices.at(vtxIdx[1]));
        mData.mEdges.push_back(edge);
    }
    // check failure
    if (aIn.failure())
        return aIn.errored("stream error");

    // face count
    int faceCount = 0;
    aIn.read(faceCount);
    if (faceCount < 0)
        return aIn.errored("invalid face count");

    for (int i = 0; i < faceCount; ++i)
    {
        int edgeIdx[3] = {};
        aIn.read(edgeIdx[0]);
        aIn.read(edgeIdx[1]);
        aIn.read(edgeIdx[2]);

        if (!xc_contains(edgeIdx[0], 0, edgeCount - 1) ||
            !xc_contains(edgeIdx[1], 0, edgeCount - 1) ||
            !xc_contains(edgeIdx[2], 0, edgeCount - 1))
        {
            return aIn.errored("invalid face reference");
        }
        auto face = new MeshFace();
        face->rawInit(
                *mData.mEdges.at(edgeIdx[0]),
                *mData.mEdges.at(edgeIdx[1]),
                *mData.mEdges.at(edgeIdx[2]));
        mData.mFaces.push_back(face);
    }
    // check failure
    if (aIn.failure())
        return aIn.errored("stream error");

    // update gl attribute
    updateGLAttribute();

    // gl initialize
    mData.getMeshBuffer();

    aIn.popLogScope();
    return aIn.checkStream();
}

cmnd::Vector MeshKey::createTrianglePusher(
        const QVector<QVector2D>& aPos,
        const QVector<MeshVtx*>& aRef,
        MeshFace** aDst)
{
    class TrianglePusher : public cmnd::Scalable
    {
        MeshKey& mKey;
        QVector<QVector2D> mPos;
        QVector<MeshVtx*> mVtxRef;
        MeshFace** mDst;
    public:
        TrianglePusher(MeshKey& aKey,
                       const QVector<QVector2D>& aPos,
                       const QVector<MeshVtx*>& aRef,
                       MeshFace** aDst)
            : mKey(aKey)
            , mPos(aPos)
            , mVtxRef(aRef)
            , mDst(aDst)
        {
        }

        virtual bool initializeAndExecute()
        {
            *mDst = mKey.createTriangle(mPos, mVtxRef, this->commands());
            return true;
        }
    };

    cmnd::Vector result;
    result.push(new TrianglePusher(*this, aPos, aRef, aDst));
    return result;
}

MeshFace* MeshKey::createTriangle(
        const QVector<QVector2D>& aPos,
        const QVector<MeshVtx*>& aRef,
        cmnd::Vector& aCommands)
{
    XC_ASSERT(aPos.count() >= 3 && aRef.count() >= 3);

    MeshVtx* vtx[3] = { aRef[0], aRef[1], aRef[2] };
    MeshEdge* edge[3] = {};

    // create vertices
    if (!vtx[0])
    {
        auto command = new MeshKeyUtil::CreateVtx(*this, mData.mVertices, aPos[0]);
        command->exec();
        aCommands.push(command);
        vtx[0] = command->newVtx();
    }
    else
    {
        if (vtx[1])
        {
            edge[0] = MeshKeyUtil::findEdge(vtx[0], vtx[1]);
        }
        if (vtx[2])
        {
            edge[2] = MeshKeyUtil::findEdge(vtx[2], vtx[0]);
        }
    }

    if (!vtx[1])
    {
        auto command = new MeshKeyUtil::CreateVtx(*this, mData.mVertices, aPos[1]);
        command->exec();
        aCommands.push(command);
        vtx[1] = command->newVtx();
    }
    else
    {
        if (vtx[2])
        {
            edge[1] = MeshKeyUtil::findEdge(vtx[1], vtx[2]);
        }
    }

    if (!vtx[2])
    {
        auto command = new MeshKeyUtil::CreateVtx(*this, mData.mVertices, aPos[2]);
        command->exec();
        aCommands.push(command);
        vtx[2] = command->newVtx();
    }

    bool allEdgeExisted = true;

    // create edges
    if (!edge[0])
    {
        auto command = new MeshKeyUtil::CreateEdge(mData.mEdges, *vtx[0], *vtx[1]);
        command->exec();
        aCommands.push(command);
        edge[0] = command->newEdge();
        allEdgeExisted = false;
    }
    if (!edge[1])
    {
        auto command = new MeshKeyUtil::CreateEdge(mData.mEdges, *vtx[1], *vtx[2]);
        command->exec();
        aCommands.push(command);
        edge[1] = command->newEdge();
        allEdgeExisted = false;
    }
    if (!edge[2])
    {
        auto command = new MeshKeyUtil::CreateEdge(mData.mEdges, *vtx[2], *vtx[0]);
        command->exec();
        aCommands.push(command);
        edge[2] = command->newEdge();
        allEdgeExisted = false;
    }

    MeshFace* face = nullptr;
    if (allEdgeExisted)
    {
        face = MeshKeyUtil::findFace(edge[0], edge[1], edge[2]);
    }

    // create face
    if (!face)
    {
        auto command = new MeshKeyUtil::CreateFace(
                           mData.mFaces, *edge[0], *edge[1], *edge[2]);
        command->exec();
        aCommands.push(command);
        face = command->newFace();
    }

    return face;
}

cmnd::Vector MeshKey::createRemover(MeshVtx& aVtx)
{
    MeshKey* key = this;
    MeshVtx* vtx = &aVtx;
    auto command = new cmnd::LambdaScalable([=](cmnd::Vector& aCommands)
    {
        key->listNeedToRemove(*vtx, aCommands);
    });

    cmnd::Vector result;
    result.push(command);
    return result;
}

void MeshKey::listNeedToRemove(MeshVtx& aStartingVtx, cmnd::Vector& aCommands)
{
    // find faces to kill
    QVector<MeshFace*> killFaces;
    for (MeshEdgeLinkNode* edgeNode = aStartingVtx.edges(); edgeNode; edgeNode = edgeNode->next)
    {
        auto edge = edgeNode->parent;
        // each face
        for (MeshFaceLinkNode* faceNode = edge->faces(); faceNode; faceNode = faceNode->next)
        {
            auto face = faceNode->parent;
            if (!killFaces.contains(face))
            {
                killFaces.push_back(face);
                aCommands.push(new MeshKeyUtil::RemoveFace(mData.mFaces, *face));
            }
        }
    }

    // find edges to kill
    QVector<MeshEdge*> killEdges;
    for (auto face : killFaces)
    {
        for (int i = 0; i < 3; ++i)
        {
            auto edge = face->edge(i);
            if (!edge->hasOtherParents(killFaces))
            {
                if (!killEdges.contains(edge))
                {
                    killEdges.push_back(edge);
                    aCommands.push(new MeshKeyUtil::RemoveEdge(mData.mEdges, *edge));
                }
            }
        }
    }

    // find vertices to kill
    QVector<MeshVtx*> killVertices;
    {
        killVertices.push_back(&aStartingVtx);
        aCommands.push(new MeshKeyUtil::RemoveVtx(
                           *this, mData.mVertices, aStartingVtx));
    }
    for (auto edge : killEdges)
    {
        for (int i = 0; i < 2; ++i)
        {
            MeshVtx* vtx = edge->vtx(i);
            if (!vtx->hasOtherParents(killEdges))
            {
                if (!killVertices.contains(vtx))
                {
                    killVertices.push_back(vtx);
                    aCommands.push(new MeshKeyUtil::RemoveVtx(
                                       *this, mData.mVertices, *vtx));
                }
            }

        }
    }
}

cmnd::Vector MeshKey::createRemover(MeshFace& aFace)
{
    MeshKey* key = this;
    MeshFace* face = &aFace;
    auto command = new cmnd::LambdaScalable([=](cmnd::Vector& aCommands)
    {
        key->listNeedToRemove(*face, aCommands);
    });

    cmnd::Vector result;
    result.push(command);
    return result;
}

void MeshKey::listNeedToRemove(MeshFace& aStartingFace, cmnd::Vector& aCommands)
{
    // kill face
    QVector<MeshFace*> killFaces;
    {
        killFaces.push_back(&aStartingFace);
        aCommands.push(new MeshKeyUtil::RemoveFace(mData.mFaces, aStartingFace));
    }

    // find edges to kill
    QVector<MeshEdge*> killEdges;
    for (int i = 0; i < 3; ++i)
    {
        auto edge = aStartingFace.edge(i);
        if (!edge->hasOtherParents(killFaces))
        {
            if (!killEdges.contains(edge))
            {
                killEdges.push_back(edge);
                aCommands.push(new MeshKeyUtil::RemoveEdge(mData.mEdges, *edge));
            }
        }

    }

    // find vertices to kill
    QVector<MeshVtx*> killVertices;
    for (auto edge : killEdges)
    {
        for (int i = 0; i < 2; ++i)
        {
            auto vtx = edge->vtx(i);
            if (!vtx->hasOtherParents(killEdges))
            {
                if (!killVertices.contains(vtx))
                {
                    killVertices.push_back(vtx);
                    aCommands.push(new MeshKeyUtil::RemoveVtx(
                                       *this, mData.mVertices, *vtx));
                }
            }
        }
    }
}

cmnd::Vector MeshKey::createSplitter(
        MeshFace& aFace, MeshEdge& aEdgeSide,
        const QVector2D& aPosOnEdge, MeshVtx** aTail)
{
    class TrianglePusher : public cmnd::Scalable
    {
        MeshKey& mKey;
        MeshFace& mFace;
        MeshEdge& mEdgeSide;
        QVector2D mPosOnEdge;
        MeshVtx** mTail;
    public:
        TrianglePusher(MeshKey& aKey, MeshFace& aFace, MeshEdge& aEdgeSide,
                       const QVector2D& aPosOnEdge, MeshVtx** aTail)
            : mKey(aKey)
            , mFace(aFace)
            , mEdgeSide(aEdgeSide)
            , mPosOnEdge(aPosOnEdge)
            , mTail(aTail)
        {
        }

        virtual bool initializeAndExecute()
        {
            *mTail = mKey.splitTriangle(mFace, mEdgeSide, mPosOnEdge, this->commands());
            return true;
        }
    };

    cmnd::Vector result;
    result.push(new TrianglePusher(*this, aFace, aEdgeSide, aPosOnEdge, aTail));
    return result;
}

cmnd::Vector MeshKey::createSplitter(
        MeshFace& aFace, MeshEdge& aEdge0, const QVector2D& aPos0,
        MeshEdge& aEdge1, const QVector2D& aPos1, MeshVtx** aTail)
{
    class TrianglePusher : public cmnd::Scalable
    {
        MeshKey& mKey;
        MeshFace& mFace;
        MeshEdge& mEdge0;
        MeshEdge& mEdge1;
        QVector2D mPos0;
        QVector2D mPos1;
        MeshVtx** mTail;
    public:
        TrianglePusher(MeshKey& aKey, MeshFace& aFace,
                       MeshEdge& aEdge0, const QVector2D& aPos0,
                       MeshEdge& aEdge1, const QVector2D& aPos1,
                       MeshVtx** aTail)
            : mKey(aKey)
            , mFace(aFace)
            , mEdge0(aEdge0)
            , mEdge1(aEdge1)
            , mPos0(aPos0)
            , mPos1(aPos1)
            , mTail(aTail)
        {
        }

        virtual bool initializeAndExecute()
        {
            *mTail = mKey.splitTriangle(
                         mFace, mEdge0, mPos0, mEdge1, mPos1, this->commands());
            return true;
        }
    };

    cmnd::Vector result;
    result.push(new TrianglePusher(
                    *this, aFace, aEdge0, aPos0, aEdge1, aPos1, aTail));
    return result;
}

MeshVtx* MeshKey::splitTriangle(
        MeshFace& aFace, MeshEdge& aEdgeSide,
        const QVector2D& aPosOnEdge, cmnd::Vector& aCommands)
{
    XC_ASSERT(aFace.has(aEdgeSide));

    QVector<MeshVtx*> oppositeVertices;
    {
        // remove faces need to split
        QVector<MeshFace*> killFaces;

        for (MeshFaceLinkNode* it = aEdgeSide.faces(); it; it = it->next)
        {
            auto face = it->parent;
            XC_PTR_ASSERT(face);

            MeshVtx* oppoVtx = face->oppositeVtx(aEdgeSide);
            XC_PTR_ASSERT(oppoVtx);
            oppositeVertices.push_back(oppoVtx);
            killFaces.push_back(face);

        }
        for (auto face : killFaces)
        {
            auto command = new MeshKeyUtil::RemoveFace(mData.mFaces, *face);
            command->exec();
            aCommands.push(command);
        }
    }

    // find flank vertices
    MeshVtx* flankVtx0 = aEdgeSide.vtx(0);
    MeshVtx* flankVtx1 = aEdgeSide.vtx(1);
    XC_PTR_ASSERT(flankVtx0);
    XC_PTR_ASSERT(flankVtx1);

    // remove edge need to split
    {
        auto command = new MeshKeyUtil::RemoveEdge(mData.mEdges, aEdgeSide);
        command->exec();
        aCommands.push(command);
    }

    // create center vertex
    MeshVtx* centerVtx = nullptr;
    {
        auto command = new MeshKeyUtil::CreateVtx(*this, mData.mVertices, aPosOnEdge);
        command->exec();
        centerVtx = command->newVtx();
        aCommands.push(command);
    }

    // create flank edge
    MeshEdge* flankEdge0 = nullptr;
    {
        auto command = new MeshKeyUtil::CreateEdge(mData.mEdges, *centerVtx, *flankVtx0);
        command->exec();
        flankEdge0 = command->newEdge();
        aCommands.push(command);
    }

    // create flank edge
    MeshEdge* flankEdge1 = nullptr;
    {
        auto command = new MeshKeyUtil::CreateEdge(mData.mEdges, *centerVtx, *flankVtx1);
        command->exec();
        flankEdge1 = command->newEdge();
        aCommands.push(command);
    }

    // create split edge and split faces
    for (auto oppoVtx : oppositeVertices)
    {
        XC_PTR_ASSERT(oppoVtx);

        // split edge
        auto edgeCommand = new MeshKeyUtil::CreateEdge(mData.mEdges, *centerVtx, *oppoVtx);
        edgeCommand->exec();
        auto centerEdge = edgeCommand->newEdge();
        aCommands.push(edgeCommand);
        XC_ASSERT(centerEdge && centerEdge->hasChildren());

        // face 0
        auto cornerEdge0 = MeshKeyUtil::findEdge(flankVtx0, oppoVtx);
        XC_PTR_ASSERT(cornerEdge0);
        auto faceCommand0 = new MeshKeyUtil::CreateFace(
                                mData.mFaces, *centerEdge, *cornerEdge0, *flankEdge0);
        faceCommand0->exec();
        aCommands.push(faceCommand0);

        // face 1
        auto cornerEdge1 = MeshKeyUtil::findEdge(flankVtx1, oppoVtx);
        XC_PTR_ASSERT(cornerEdge1);
        auto faceCommand1 = new MeshKeyUtil::CreateFace(
                                mData.mFaces, *centerEdge, *cornerEdge1, *flankEdge1);
        faceCommand1->exec();
        aCommands.push(faceCommand1);
    }

    return centerVtx;
}

MeshVtx* MeshKey::splitTriangle(
        MeshFace& aFace, MeshEdge& aEdge0, const QVector2D& aPos0,
        MeshEdge& aEdge1, const QVector2D& aPos1, cmnd::Vector& aCommands)
{
    XC_ASSERT(aFace.has(aEdge0));
    XC_ASSERT(aFace.has(aEdge1));
    std::array<MeshEdge*, 2> edges = { &aEdge0, &aEdge1 };
    std::array<QVector2D, 2> poses = { aPos0, aPos1 };

    // common vertex of edges
    MeshVtx* commonVtx = MeshKeyUtil::findCommonVtx(aEdge0, aEdge1);
    XC_PTR_ASSERT(commonVtx);
    MeshEdge* anotherEdge = aFace.oppositeEdge(*commonVtx);
    XC_PTR_ASSERT(anotherEdge);


    QVector<MeshVtx*> oppositeVertices[2];
    {
        // remove faces need to split
        QVector<MeshFace*> killFaces;

        for (int i = 0; i < 2; ++i)
        {
            for (MeshFaceLinkNode* it = edges[i]->faces(); it; it = it->next)
            {
                auto face = it->parent;
                XC_PTR_ASSERT(face);
                if (face == &aFace) continue;

                MeshVtx* oppoVtx = face->oppositeVtx(*edges[i]);
                XC_PTR_ASSERT(oppoVtx);
                oppositeVertices[i].push_back(oppoVtx);
                if (!killFaces.contains(face))
                {
                    killFaces.push_back(face);
                }
            }
        }
        killFaces.push_back(&aFace);
        for (auto face : killFaces)
        {
            auto command = new MeshKeyUtil::RemoveFace(mData.mFaces, *face);
            command->exec();
            aCommands.push(command);
        }
    }

    // find flank vertices
    MeshVtx* flankVtx[2];
    for (int i = 0; i < 2; ++i)
    {
        flankVtx[i] = (commonVtx == edges[i]->vtx(0)) ?
                          edges[i]->vtx(1) : edges[i]->vtx(0);
        XC_PTR_ASSERT(flankVtx[i]);
    }

    // remove edge need to split
    for (int i = 0; i < 2; ++i)
    {
        auto command = new MeshKeyUtil::RemoveEdge(mData.mEdges, *edges[i]);
        command->exec();
        aCommands.push(command);
    }

    MeshVtx* centerVtx[2] = {};
    MeshEdge* flankEdgeA[2] = {};
    MeshEdge* flankEdgeB[2] = {};
    for (int i = 0; i < 2; ++i)
    {
        // create center vertex
        {
            auto command = new MeshKeyUtil::CreateVtx(*this, mData.mVertices, poses[i]);
            command->exec();
            centerVtx[i] = command->newVtx();
            aCommands.push(command);
        }

        // create flank edge
        {
            auto command = new MeshKeyUtil::CreateEdge(
                               mData.mEdges, *centerVtx[i], *flankVtx[i]);
            command->exec();
            flankEdgeA[i] = command->newEdge();
            aCommands.push(command);
        }

        // create flank edge
        {
            auto command = new MeshKeyUtil::CreateEdge(
                               mData.mEdges, *centerVtx[i], *commonVtx);
            command->exec();
            flankEdgeB[i] = command->newEdge();
            aCommands.push(command);
        }
    }

    // create split edge and split faces
    for (int i = 0; i < 2; ++i)
    {
        for (auto oppoVtx : oppositeVertices[i])
        {
            XC_PTR_ASSERT(oppoVtx);

            // split edge
            auto edgeCommand = new MeshKeyUtil::CreateEdge(
                                   mData.mEdges, *centerVtx[i], *oppoVtx);
            edgeCommand->exec();
            auto centerEdge = edgeCommand->newEdge();
            aCommands.push(edgeCommand);
            XC_ASSERT(centerEdge);

            // face 0
            auto cornerEdge0 = MeshKeyUtil::findEdge(flankVtx[i], oppoVtx);
            XC_PTR_ASSERT(cornerEdge0);
            auto faceCommand0 = new MeshKeyUtil::CreateFace(
                                    mData.mFaces, *centerEdge, *cornerEdge0, *flankEdgeA[i]);
            faceCommand0->exec();
            aCommands.push(faceCommand0);

            // face 1
            auto cornerEdge1 = MeshKeyUtil::findEdge(commonVtx, oppoVtx);
            XC_PTR_ASSERT(cornerEdge1);
            auto faceCommand1 = new MeshKeyUtil::CreateFace(
                                    mData.mFaces, *centerEdge, *cornerEdge1, *flankEdgeB[i]);
            faceCommand1->exec();
            aCommands.push(faceCommand1);
        }
    }

    // create center face
    {
        // create center edge
        MeshEdge* centerEdge = nullptr;
        {
            auto command = new MeshKeyUtil::CreateEdge(
                               mData.mEdges, *centerVtx[0], *centerVtx[1]);
            command->exec();
            centerEdge = command->newEdge();
            aCommands.push(command);
        }
        // create append edge
        MeshEdge* appendEdge = nullptr;
        {
            auto command = new MeshKeyUtil::CreateEdge(
                               mData.mEdges, *centerVtx[0], *flankVtx[1]);
            command->exec();
            appendEdge = command->newEdge();
            aCommands.push(command);
        }
        // create face A
        {
            auto command = new MeshKeyUtil::CreateFace(
                               mData.mFaces, *flankEdgeB[0],
                               *flankEdgeB[1], *centerEdge);
            command->exec();
            aCommands.push(command);
        }
        // create face B
        {
            auto command = new MeshKeyUtil::CreateFace(
                               mData.mFaces, *centerEdge,
                               *flankEdgeA[1], *appendEdge);
            command->exec();
            aCommands.push(command);
        }
        // create face C
        {
            auto command = new MeshKeyUtil::CreateFace(
                               mData.mFaces, *flankEdgeA[0],
                               *appendEdge, *anotherEdge);
            command->exec();
            aCommands.push(command);
        }
    }

    return centerVtx[1];
}

void MeshKey::moveVtx(MeshVtx& aVtx, const QVector2D& aPos)
{
    aVtx.set(aPos);
    const int index = mData.mVertices.indexOf(&aVtx);
    XC_ASSERT(index >= 0);
    mData.mPositions[index].set(aPos.toVector3D());
}

void MeshKey::updateVtxIndices()
{
    int vtxCount = 0;
    for (auto vtx : mData.mVertices)
    {
        vtx->setIndex(vtxCount);
        ++vtxCount;
    }
}

void MeshKey::updateGLAttribute()
{
    int vtxCount = mData.mVertices.count();

    mData.mPositions.resize(vtxCount);
    mData.mTexCoords.resize(vtxCount);

    {
        int i = 0;
        for (auto vtx : mData.mVertices)
        {
            XC_PTR_ASSERT(vtx);
            auto vec = vtx->vec();
            mData.mPositions[i].set(vec.x(), vec.y(), 0.0f);
            mData.mTexCoords[i].set(vec.x(), vec.y());
            ++i;
        }
    }

    const int idxCount = mData.mFaces.count() * 3;
    mData.mIndices.resize(idxCount);

    {
        int i = 0;
        for (auto face : mData.mFaces)
        {
            XC_PTR_ASSERT(face);
            auto vtx = face->vertices();
            mData.mIndices[i    ] = vtx[0]->index();
            mData.mIndices[i + 1] = vtx[1]->index();
            mData.mIndices[i + 2] = vtx[2]->index();
            i += 3;
        }
    }
}

} // namespace core

