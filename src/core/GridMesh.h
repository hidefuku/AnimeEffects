#ifndef CORE_GRIDMESH_H
#define CORE_GRIDMESH_H

#include <QGL>
#include <QSize>
#include <QScopedArrayPointer>
#include "XC.h"
#include "util/Dir4.h"
#include "util/Triangle2DPos.h"
#include "util/ArrayBlock.h"
#include "gl/Vector2.h"
#include "gl/Vector3.h"
#include "img/GridMeshCreator.h"
#include "core/LayerMesh.h"
#include "core/Serializer.h"
#include "core/Deserializer.h"

namespace core
{
class HeightMap;

class GridMesh : public LayerMesh
{
public:
    typedef std::array<GLuint, 3> TriId;

    struct Transition
    {
        Transition() : id(), pos() {}
        TriId id;
        util::Triangle2DPos pos;
    };

    struct Transitions
    {
        QVector<Transition> data;
        QVector2D offset;
    };

    class TransitionCreater
    {
        QRect mRect;
        int mIndexCount;
        QScopedArrayPointer<GLuint> mIndices;
        QScopedArrayPointer<gl::Vector3> mPositions;
        QPoint mTopLeft;
        bool mQuad;

    public:
        TransitionCreater(const GridMesh& aPrev, const QPoint& aTopLeft);
        Transitions create(
                const gl::Vector3* aNext, int aCount,
                const QPoint& aTopLeft);
    };

    GridMesh();
    ~GridMesh();

    void createFromImage(const void* aImagePtr, const QSize& aSize, int aCellPx);
    void writeHeightMap(const HeightMap& aMap, const QVector2D& aMinPos);
    util::ArrayBlock<gl::Vector3> createFFD(
            util::ArrayBlock<const gl::Vector3> aPrevFFD,
            const Transitions& aTrans) const;

    // from LayerMesh
    virtual GLenum primitiveMode() const { return mPrimitiveType; }
    virtual const gl::Vector3* positions() const { return mPositions.data(); }
    virtual const gl::Vector2* texCoords() const { return mTexCoords.data(); }
    virtual int vertexCount() const { return mVertexCount; }
    virtual const GLuint* indices() const { return mIndices.data(); }
    virtual GLsizei indexCount() const { return (GLsizei)mIndexCount; }
    virtual MeshBuffer& getMeshBuffer();
    virtual void resetArrayedConnection(
            ArrayedConnectionList& aDest,
            const gl::Vector3* aPositions) const;
    virtual Frame frameSign() const { return Frame(-1); }

    const gl::Vector3* offsets() const { return mOffsets.data(); }
    const gl::Vector3* normals() const { return mNormals.data(); }

    const QSize& size() const { return mSize; }
    int cellSize() const { return mCellPx; }
    const QRect& vertexRect() const { return mVertexRect; }

    bool serialize(Serializer& aOut) const;
    bool deserialize(Deserializer& aIn);

private:
    struct QuadConnection
    {
        int id[util::Dir4_TERM];
        bool has(int aIndex) const { return id[aIndex] != -1; }
        void setUp(int aId) { id[util::Dir4_Up] = aId; }
        void setDown(int aId) { id[util::Dir4_Down] = aId; }
        void setLeft(int aId) { id[util::Dir4_Left] = aId; }
        void setRight(int aId) { id[util::Dir4_Right] = aId; }
        void clear();
    };
    typedef img::GridMeshCreator::HexaConnection HexaConnection;

    enum { kMaxConnectionCount = 6, kHexaConnectionCount = 6, kQuadConnectionCount = 4 };

    void createFromImageVer1(const void* aImagePtr, const QSize& aSize, int aCellPx);
    void createFromImageVer2(const void* aImagePtr, const QSize& aSize, int aCellPx);

    void allocIndexBuffer(int aIndexCount);
    void allocVertexBuffers(int aVertexCount);
    void initializeVertexBuffers(int aVertexCount);
    void freeBuffers();
    std::pair<bool, gl::Vector3> gatherValidPositions(
            int aIndex, const gl::Vector3* aPositions,
            const bool* aValidity) const;
    bool hasConnection(int aArrayIndex, int aIdIndex) const;
    int connectionId(int aArrayIndex, int aIdIndex) const;

    QSize mSize;
    int mCellNumX;
    int mCellNumY;
    int mCellPx;
    GLenum mPrimitiveType;
    int mIndexCount;
    int mVertexCount;
    QRect mVertexRect;
    QScopedArrayPointer<GLuint> mIndices;
    QScopedArrayPointer<gl::Vector3> mPositions;
    QScopedArrayPointer<gl::Vector3> mOffsets;
    QScopedArrayPointer<gl::Vector2> mTexCoords;
    QScopedArrayPointer<gl::Vector3> mNormals;
    QScopedArrayPointer<QuadConnection> mQuadConnections;
    QScopedArrayPointer<HexaConnection> mHexaConnections;
    QScopedPointer<MeshBuffer> mMeshBuffer;
};

} // namespace core

#endif // CORE_GRIDMESH_H
