#ifndef CTRL_MESH_MESHACCESSOR_H
#define CTRL_MESH_MESHACCESSOR_H

#include "cmnd/Vector.h"
#include "core/MeshKey.h"

namespace ctrl {
namespace mesh {

typedef core::MeshVtx MeshVtx;
typedef core::MeshEdge MeshEdge;
typedef core::MeshFace MeshFace;
typedef core::MeshEdgeLinkNode MeshEdgeLinkNode;
typedef core::MeshFaceLinkNode MeshFaceLinkNode;

//-------------------------------------------------------------------------------------------------
class MeshAccessor
{
public:
    typedef core::MeshKey::Data::VtxList VtxList;
    typedef core::MeshKey::Data::EdgeList EdgeList;
    typedef core::MeshKey::Data::FaceList FaceList;

    MeshAccessor();

    const VtxList& vertices() const { return mKey->data().vertices(); }
    const EdgeList& edges() const { return mKey->data().edges(); }
    const FaceList& faces() const { return mKey->data().faces(); }

    void setKey(core::MeshKey& aKey)
    {
        mKey = &aKey;
    }

    cmnd::Vector createTrianglePusher(
            const QVector<QVector2D>& aPos,
            const QVector<MeshVtx*>& aRef,
            MeshFace** aDst)
    {
        return mKey->createTrianglePusher(aPos, aRef, aDst);
    }

    cmnd::Vector createRemover(MeshVtx& aVtx)
    {
        return mKey->createRemover(aVtx);
    }

    cmnd::Vector createRemover(MeshFace& aFace)
    {
        return mKey->createRemover(aFace);
    }

    cmnd::Vector createSplitter(
            MeshFace& aFace, MeshEdge& aEdgeSide,
            const QVector2D& aPosOnEdge, MeshVtx** aTail)
    {
        return mKey->createSplitter(aFace, aEdgeSide, aPosOnEdge, aTail);
    }

    cmnd::Vector createSplitter(
            MeshFace& aFace, MeshEdge& aEdge0, const QVector2D& aPos0,
            MeshEdge& aEdge1, const QVector2D& aPos1, MeshVtx** aTail)
    {
        return mKey->createSplitter(aFace, aEdge0, aPos0, aEdge1, aPos1, aTail);
    }

private:
    core::MeshKey* mKey;
};

} // namespace mesh
} // namespace ctrl

#endif // CTRL_MESH_MESHACCESSOR_H
