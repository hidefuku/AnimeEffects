#ifndef CORE_GRIDMESH_H
#define CORE_GRIDMESH_H

#include <QGL>
#include <QSize>
#include <QScopedArrayPointer>
#include "XC.h"
#include "util/Dir4.h"
#include "util/Triangle2DPos.h"
#include "util/ArrayBlock.h"
#include "util/ArrayBuffer.h"
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

    public:
        TransitionCreater(
                const GridMesh& aPrev,
                const QPoint& aTopLeft);
        Transitions create(
                const gl::Vector3* aNext, int aCount,
                const QPoint& aTopLeft);
    };

    GridMesh();
    GridMesh(const GridMesh& aRhs);
    GridMesh& operator=(const GridMesh& aRhs);
    ~GridMesh();

    void swap(GridMesh& aRhs);
    void setOriginOffset(const QVector2D& aOffset) { mOriginOffset = aOffset; }
    void createFromImage(const void* aImagePtr, const QSize& aSize, int aCellPx);
    void writeHeightMap(const HeightMap& aMap, const QVector2D& aMinPos);
    util::ArrayBlock<gl::Vector3> createFFD(
            util::ArrayBlock<const gl::Vector3> aPrevFFD,
            const Transitions& aTrans) const;

    // from LayerMesh
    virtual GLenum primitiveMode() const { return GL_TRIANGLES; }
    virtual const gl::Vector3* positions() const { return mPositions.data(); }
    virtual const gl::Vector2* texCoords() const { return mTexCoords.data(); }
    virtual int vertexCount() const { return mVertexCount; }
    virtual const GLuint* indices() const { return mIndices.data(); }
    virtual GLsizei indexCount() const { return (GLsizei)mIndexCount; }
    virtual MeshBuffer& getMeshBuffer();
    virtual void resetArrayedConnection(
            ArrayedConnectionList& aDest,
            const gl::Vector3* aPositions) const;
    virtual Frame frameSign() const;
    virtual QVector2D originOffset() const { return mOriginOffset; }

    const gl::Vector3* offsets() const { return mOffsets.data(); }
    const gl::Vector3* normals() const { return mNormals.data(); }

    const QSize& size() const { return mSize; }
    int cellSize() const { return mCellPx; }
    const QRect& vertexRect() const { return mVertexRect; }

    bool serialize(Serializer& aOut) const;
    bool deserialize(Deserializer& aIn);

private:
    typedef img::GridMeshCreator::HexaConnection HexaConnection;
    enum { kMaxConnectionCount = 6, kHexaConnectionCount = 6 };

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
    QVector2D mOriginOffset;
    int mCellPx;
    int mIndexCount;
    int mVertexCount;
    QRect mVertexRect;
    util::ArrayBuffer<GLuint> mIndices;
    util::ArrayBuffer<gl::Vector3> mPositions;
    util::ArrayBuffer<gl::Vector3> mOffsets;
    util::ArrayBuffer<gl::Vector2> mTexCoords;
    util::ArrayBuffer<gl::Vector3> mNormals;
    util::ArrayBuffer<HexaConnection> mHexaConnections;
    QScopedPointer<MeshBuffer> mMeshBuffer;
};

} // namespace core

#endif // CORE_GRIDMESH_H
