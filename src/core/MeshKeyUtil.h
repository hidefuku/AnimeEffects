#ifndef CORE_MESHKEYUTIL_H
#define CORE_MESHKEYUTIL_H

#include "cmnd/Stable.h"
#include "cmnd/UndoneDeleter.h"
#include "cmnd/DoneDeleter.h"
#include "core/MeshKey.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
class MeshKeyUtil
{
public:

    //-------------------------------------------------------------------------------------------------
    class CreateFace : public cmnd::Stable
    {
        QList<MeshFace*>& mFaceList;
        cmnd::UndoneDeleter<MeshFace> mNewFace;
        std::array<MeshEdge*, 3> mEdges;

    public:
        CreateFace(QList<MeshFace*>& aFaceList,
                    MeshEdge& aEdge0,
                    MeshEdge& aEdge1,
                    MeshEdge& aEdge2);

        MeshFace* newFace() { return mNewFace.get(); }

        virtual void exec();
        virtual void redo();
        virtual void undo();
    };

    //-------------------------------------------------------------------------------------------------
    class CreateEdge : public cmnd::Stable
    {
        QList<MeshEdge*>& mEdgeList;
        cmnd::UndoneDeleter<MeshEdge> mNewEdge;
        std::array<MeshVtx*, 2> mVtxs;

    public:
        CreateEdge(QList<MeshEdge*>& aEdgeList,
                    MeshVtx& aVtx0, MeshVtx& aVtx1);

        MeshEdge* newEdge() { return mNewEdge.get(); }

        virtual void exec();
        virtual void redo();
        virtual void undo();
    };

    //-------------------------------------------------------------------------------------------------
    class CreateVtx : public cmnd::Stable
    {
        MeshKey& mKey; // for ffd key
        QList<MeshVtx*>& mVtxList;
        cmnd::UndoneDeleter<MeshVtx> mNewVtx;
        QVector2D mPos;

    public:
        CreateVtx(MeshKey& aKey, QList<MeshVtx*>& aVtxList, const QVector2D& aPos);

        MeshVtx* newVtx() { return mNewVtx.get(); }

        virtual void exec();
        virtual void redo();
        virtual void undo();

    };

    //-------------------------------------------------------------------------------------------------
    class RemoveFace : public cmnd::Stable
    {
        QList<MeshFace*>& mFaceList;
        cmnd::DoneDeleter<MeshFace> mDelFace;
        std::array<MeshEdge*, 3> mPrevEdges;
        int mIndex;

    public:
        RemoveFace(QList<MeshFace*>& aFaceList, MeshFace& aDelFace);

        virtual void exec();
        virtual void redo();
        virtual void undo();
    };

    //-------------------------------------------------------------------------------------------------
    class RemoveEdge : public cmnd::Stable
    {
        QList<MeshEdge*>& mEdgeList;
        cmnd::DoneDeleter<MeshEdge> mDelEdge;
        std::array<MeshVtx*, 2> mPrevVtxs;
        int mIndex;

    public:
        RemoveEdge(QList<MeshEdge*>& aEdgeList, MeshEdge& aDelEdge);

        virtual void exec();
        virtual void redo();
        virtual void undo();
    };

    //-------------------------------------------------------------------------------------------------
    class RemoveVtx : public cmnd::Stable
    {
        MeshKey& mKey; // for ffd key, for update vertex indices
        QList<MeshVtx*>& mVtxList;
        cmnd::DoneDeleter<MeshVtx> mDelVtx;
        QVector<gl::Vector3> mPrevFFDs;
        int mIndex;

    public:
        RemoveVtx(MeshKey& aKey, QList<MeshVtx*>& aVtxList, MeshVtx& aDelVtx);

        virtual void exec();
        virtual void redo();
        virtual void undo();
    };

    //-------------------------------------------------------------------------------------------------
    static MeshEdge* findEdge(const MeshVtx* aVtx0, const MeshVtx* aVtx1);
    static MeshFace* findFace(const MeshEdge* aEdge0, const MeshEdge* aEdge1, const MeshEdge* aEdge2);
    static MeshVtx* findCommonVtx(const MeshEdge& aEdge0, const MeshEdge& aEdge1);
};

} // namespace core

#endif // CORE_MESHKEYUTIL_H
