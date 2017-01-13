#ifndef CORE_MESHKEY_H
#define CORE_MESHKEY_H

#include <vector>
#include <array>
#include <QList>
#include <QVector>
#include <QVector2D>
#include "util/NonCopyable.h"
#include "util/Segment2D.h"
#include "cmnd/Vector.h"
#include "core/TimeKey.h"
#include "core/LayerMesh.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
template <typename tParent, typename tChild>
struct MeshLinkNode
{
    MeshLinkNode()
        : parent(), child(), prev(), next() {}

    tParent* parent;
    tChild* child;
    MeshLinkNode<tParent, tChild>* prev;
    MeshLinkNode<tParent, tChild>* next;
};

class MeshVtx;
class MeshEdge;
class MeshFace;

typedef MeshLinkNode<MeshEdge, MeshVtx> MeshEdgeLinkNode;
typedef MeshLinkNode<MeshFace, MeshEdge> MeshFaceLinkNode;
typedef MeshEdgeLinkNode* MeshEdgeLink;
typedef MeshFaceLinkNode* MeshFaceLink;

//-------------------------------------------------------------------------------------------------
class MeshVtx : private util::NonCopyable
{
public:
    float x;
    float y;

    MeshVtx();
    MeshVtx(float aX, float aY);
    ~MeshVtx();
    MeshVtx(const QVector2D& aPos);
    QVector2D vec() const { return QVector2D(x, y); }
    MeshEdgeLink edges() const { return mEdges; }
    void set(float aX, float aY) { x = aX; y = aY; }
    void set(const QVector2D& aPos) { x = aPos.x(); y = aPos.y(); }
    bool hasOtherParents(const QVector<MeshEdge*>& aEdges) const;
    bool hasParent() const { return mEdges; }
    void setIndex(int aIndex) { mIndex = aIndex; }
    int index() const { return mIndex; }
    int connectionCount() const;

private:
    friend class MeshEdge;
    void link(MeshEdgeLinkNode& aEdge);
    void unlink(MeshEdgeLinkNode& aEdge);

    MeshEdgeLink mEdges;
    int mIndex;
};

//-------------------------------------------------------------------------------------------------
class MeshEdge : private util::NonCopyable
{
public:
    MeshEdge();
    ~MeshEdge();

    void set(MeshVtx& aVp0, MeshVtx& aVp1);
    void clear();

    inline QVector2D start() const { return mLink[0].child->vec(); }
    inline QVector2D end() const { return mLink[1].child->vec(); }
    inline QVector2D dir() const { return end() - start(); }
    inline util::Segment2D seg() const { return util::Segment2D(start(), dir()); }
    inline MeshVtx* vtx(int aIndex) const { return mLink[aIndex].child; }
    MeshFaceLink faces() const { return mFaces; }
    bool isEquals(const MeshVtx& aVp0, const MeshVtx& aVp1) const;
    bool has(const MeshVtx& aVtx) const { return vtx(0) == &aVtx || vtx(1) == &aVtx; }
    bool hasParent() const { return mFaces; }
    bool hasChildren() const { return mLink[0].child || mLink[1].child; }
    bool hasMultiParents() const;
    bool hasOtherParents(const QVector<MeshFace*>& aFaces) const;

    // for deserialize
    void rawInit(MeshVtx& aVp0, MeshVtx& aVp1);

private:
    friend class MeshFace;
    friend class MeshVtx;

    void link(MeshFaceLinkNode& aFace);
    void unlink(MeshFaceLinkNode& aFace);

    MeshEdgeLinkNode mLink[2];
    MeshFaceLink mFaces;

};

//-------------------------------------------------------------------------------------------------
class MeshFace : private util::NonCopyable
{
public:
    MeshFace();
    ~MeshFace();

    void set(MeshEdge& aEp0, MeshEdge& aEp1, MeshEdge& aEp2);
    void clear();

    inline MeshEdge* edge(int aIndex) const { return mLink[aIndex].child; }
    MeshEdge* findEdge(const MeshVtx& aVtx0, const MeshVtx& aVtx1) const;
    std::array<MeshVtx*, 3> vertices() const;
    QVector2D vnorm(int aIndex) const;
    bool has(const MeshEdge& aEdge) const;
    bool has(const MeshVtx& aVtx) const;
    bool hasChildren() const;
    bool isClockwise() const;
    bool containsPoint(const QVector2D& aPoint) const;
    MeshEdge* nearestEdge(const QVector2D& aPoint) const;
    MeshVtx* oppositeVtx(const MeshEdge& aEdge) const;
    MeshEdge* oppositeEdge(const MeshVtx& aVtx) const;

    void rawInit(MeshEdge& aEp0, MeshEdge& aEp1, MeshEdge& aEp2);

private:
    void updateDir();
    friend class MeshEdge;

    MeshFaceLinkNode mLink[3];
    bool mInverse[3];
    bool mClockwise;
};

//-------------------------------------------------------------------------------------------------
class MeshKey : public TimeKey
{
public:
    class Data : public LayerMesh
    {
    public:
        typedef QList<MeshVtx*> VtxList;
        typedef QList<MeshEdge*> EdgeList;
        typedef QList<MeshFace*> FaceList;
        Data();
        Data(const Data& aRhs);
        Data& operator=(const Data& aRhs);
        ~Data();

        const VtxList& vertices() const { return mVertices; }
        const EdgeList& edges() const { return mEdges; }
        const FaceList& faces() const { return mFaces; }

        void setOriginOffset(const QVector2D& aOffset) { mOriginOffset = aOffset; }

        // from LayerMesh
        virtual GLenum primitiveMode() const { return GL_TRIANGLES; }
        virtual const gl::Vector3* positions() const { return mPositions.data(); }
        virtual const gl::Vector2* texCoords() const { return mTexCoords.data(); }
        virtual int vertexCount() const { return mPositions.count(); }
        virtual const GLuint* indices() const { return mIndices.data(); }
        virtual GLsizei indexCount() const { return mIndices.count(); }
        virtual gl::BufferObject& getIndexBuffer();
        virtual MeshBuffer& getMeshBuffer();
        virtual void resetArrayedConnection(
                ArrayedConnectionList& aDest,
                const gl::Vector3* aPositions) const;
        virtual Frame frameSign() const { return Frame(mOwner->frame()); }
        virtual QVector2D originOffset() const { return mOriginOffset; }

        void destroy();

    private:
        friend class MeshKey;

        void copyVerticesEdgesAndFaces(const Data& aRhs);
        void updateVtxIndices();
        void updateGLAttribute();
        void resetIndexBuffer();
        bool serialize(Serializer& aOut) const;
        bool deserialize(Deserializer& aIn);

        QVector2D mOriginOffset;
        VtxList mVertices;
        EdgeList mEdges;
        FaceList mFaces;
        QVector<gl::Vector3> mPositions;
        QVector<gl::Vector2> mTexCoords;
        QVector<GLuint> mIndices;
        QScopedPointer<MeshBuffer> mMeshBuffer;
        QScopedPointer<gl::BufferObject> mIndexBuffer;
        MeshKey* mOwner;
    };

    MeshKey();

    Data& data() { return mData; }
    const Data& data() const { return mData; }

    ///@attention "aDst" will be assigned when the command executed.
    cmnd::Vector createTrianglePusher(
            const QVector<QVector2D>& aPos,
            const QVector<MeshVtx*>& aRef,
            MeshFace** aDst);
    cmnd::Vector createRemover(MeshVtx& aVtx);
    cmnd::Vector createRemover(MeshFace& aFace);
    cmnd::Vector createSplitter(
            MeshFace& aFace, MeshEdge& aEdgeSide,
            const QVector2D& aPosOnEdge, MeshVtx** aTail);
    cmnd::Vector createSplitter(
            MeshFace& aFace, MeshEdge& aEdge0, const QVector2D& aPos0,
            MeshEdge& aEdge1, const QVector2D& aPos1, MeshVtx** aTail);
    void moveVtx(MeshVtx& aVtx, const QVector2D& aPos);
    void updateGLAttribute();

    virtual TimeKeyType type() const { return TimeKeyType_Mesh; }
    virtual bool canHoldChild() const { return true; }
    virtual TimeKey* createClone();
    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

public:
    void updateVtxIndices();

private:
    MeshFace* createTriangle(
            const QVector<QVector2D>& aPos,
            const QVector<MeshVtx*>& aRef,
            cmnd::Vector& aCommands);
    void listNeedToRemove(MeshVtx& aStartingVtx, cmnd::Vector& aCommands);
    void listNeedToRemove(MeshFace& aStartingFace, cmnd::Vector& aCommands);
    MeshVtx* splitTriangle(
            MeshFace& aFace, MeshEdge& aEdgeSide,
            const QVector2D& aPosOnEdge, cmnd::Vector& aCommands);
    MeshVtx* splitTriangle(
            MeshFace& aFace, MeshEdge& aEdge0, const QVector2D& aPos0,
            MeshEdge& aEdge1, const QVector2D& aPos1, cmnd::Vector& aCommands);

    Data mData;
};

} // namespace core

#endif // CORE_MESHKEY_H
