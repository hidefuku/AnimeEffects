#include <float.h>
#include <QVector3D>
#include <QMatrix4x4>
#include <QPoint>
#include <QtMath>
#include <QScopedArrayPointer>
#include "util/TriangleRasterizer.h"
#include "util/CollDetect.h"
#include "util/MathUtil.h"
#include "util/BinarySpacePartition2D.h"
#include "img/PixelPos.h"
#include "img/Quad.h"
#include "core/GridMesh.h"
#include "core/HeightMap.h"
#include "core/TimeLine.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
GridMesh::GridMesh()
    : mSize()
    , mOriginOffset()
    , mCellPx(0)
    , mIndexCount(0)
    , mVertexCount(0)
    , mVertexRect()
    , mIndices()
    , mPositions()
    , mOffsets()
    , mTexCoords()
    , mNormals()
    , mHexaConnections()
    , mMeshBuffer()
    , mIndexBuffer()
{
    // initialize mesh buffer
    getMeshBuffer();
}

GridMesh::GridMesh(const GridMesh& aRhs)
    : mSize(aRhs.mSize)
    , mOriginOffset(aRhs.mOriginOffset)
    , mCellPx(aRhs.mCellPx)
    , mIndexCount(aRhs.mIndexCount)
    , mVertexCount(aRhs.mVertexCount)
    , mVertexRect(aRhs.mVertexRect)
    , mIndices(aRhs.mIndices)
    , mPositions(aRhs.mPositions)
    , mOffsets(aRhs.mOffsets)
    , mTexCoords(aRhs.mTexCoords)
    , mNormals(aRhs.mNormals)
    , mHexaConnections(aRhs.mHexaConnections)
    , mMeshBuffer()
    , mIndexBuffer()
{
    // initialize mesh buffer
    getMeshBuffer();
}

GridMesh& GridMesh::operator=(const GridMesh& aRhs)
{
    freeBuffers();

    mSize = aRhs.mSize;
    mOriginOffset = aRhs.mOriginOffset;
    mCellPx = aRhs.mCellPx;
    mIndexCount = aRhs.mIndexCount;
    mVertexCount = aRhs.mVertexCount;
    mVertexRect = aRhs.mVertexRect;
    mIndices = aRhs.mIndices;
    mPositions = aRhs.mPositions;
    mOffsets = aRhs.mOffsets;
    mTexCoords = aRhs.mTexCoords;
    mNormals = aRhs.mNormals;
    mHexaConnections = aRhs.mHexaConnections;

    // reset index buffer
    resetIndexBuffer();

    // initialize mesh buffer
    getMeshBuffer();

    return *this;
}

GridMesh::~GridMesh()
{
    freeBuffers();
}

void GridMesh::swap(GridMesh& aRhs)
{
    std::swap(mSize, aRhs.mSize);
    std::swap(mOriginOffset, aRhs.mOriginOffset);
    std::swap(mCellPx, aRhs.mCellPx);
    std::swap(mIndexCount, aRhs.mIndexCount);
    std::swap(mVertexCount, aRhs.mVertexCount);
    std::swap(mVertexRect, aRhs.mVertexRect);
    mIndices.swap(aRhs.mIndices);
    mPositions.swap(aRhs.mPositions);
    mOffsets.swap(aRhs.mOffsets);
    mTexCoords.swap(aRhs.mTexCoords);
    mNormals.swap(aRhs.mNormals);
    mHexaConnections.swap(aRhs.mHexaConnections);
    mMeshBuffer.swap(aRhs.mMeshBuffer);

    resetIndexBuffer();
    aRhs.resetIndexBuffer();
}

void GridMesh::freeBuffers()
{
    mPositions.reset();
    mOffsets.reset();
    mTexCoords.reset();
    mNormals.reset();
    mHexaConnections.reset();
    mIndices.reset();
    resetIndexBuffer();
}

void GridMesh::resetIndexBuffer()
{
    if (mIndices && mIndexCount > 0)
    {
        if (!mIndexBuffer)
        {
            mIndexBuffer.reset(new gl::BufferObject(GL_ELEMENT_ARRAY_BUFFER));
        }
        mIndexBuffer->resetData(mIndexCount, GL_STATIC_DRAW, mIndices.data());
    }
    else
    {
        mIndexBuffer.reset();
    }
}

void GridMesh::allocVertexBuffers(int aVertexCount)
{
    mPositions.construct(aVertexCount);
    mOffsets.construct(aVertexCount);
    mTexCoords.construct(aVertexCount);
    mNormals.construct(aVertexCount);
    mHexaConnections.construct(aVertexCount);
}

void GridMesh::allocIndices(int aIndexCount)
{
    mIndices.construct(aIndexCount);
}

void GridMesh::initializeVertexBuffers(int aVertexCount)
{
    for (int i = 0; i < aVertexCount; ++i)
    {
        mPositions[i].setZero();
        mTexCoords[i].setZero();
        mOffsets[i].setZero();
        mNormals[i].set(0.0f, 0.0f, 1.0f);
    }
}

void GridMesh::createFromImage(const void* aImagePtr, const QSize& aSize, int aCellPx)
{
    XC_ASSERT(aCellPx > 0);

    // free old buffers
    freeBuffers();

    mSize = aSize;
    mCellPx = aCellPx;

    if (!aImagePtr || aSize.isEmpty() || (aSize == QSize(1, 1) && *((uint32*)aImagePtr) == 0))
    {
        mVertexRect = QRect(QPoint(), QSize());
        mVertexCount = 0;
        mIndexCount = 0;
    }
    else if (aCellPx < aSize.width() - 2 || aCellPx < aSize.height() - 2)
    {
        createGridMesh(aImagePtr);
    }
    else
    {
        createQuadMesh(); // create bounding quadangle
    }
}

void GridMesh::createGridMesh(const void* aImagePtr)
{
    img::GridMeshCreator creator((const uint8*)aImagePtr, mSize, mCellPx);

    mVertexRect = creator.vertexRect();
    mVertexCount = creator.vertexCount();
    mIndexCount = creator.indexCount();

    // return if any vertices does not exist
    if (mVertexCount <= 0 || mIndexCount <= 0) return;

    // allocate attribute buffers
    allocIndices(mIndexCount);
    allocVertexBuffers(mVertexCount);
    initializeVertexBuffers(mVertexCount);

    // setup indices
    creator.writeIndices(mIndices.data());
    // update gl index buffer
    resetIndexBuffer();

    // setup attributes
    creator.writeVertices((GLfloat*)mPositions.data(), (GLfloat*)mTexCoords.data());

    // setup connections
    creator.writeConnections(mHexaConnections.data());

}

void GridMesh::createQuadMesh()
{
    mVertexRect = QRect(QPoint(), mSize);
    mVertexCount = 4;
    mIndexCount = 6;

    // allocate attribute buffers
    allocIndices(mIndexCount);
    allocVertexBuffers(mVertexCount);
    initializeVertexBuffers(mVertexCount);

    // indices
    mIndices[0] = 0; mIndices[1] = 1; mIndices[2] = 2;
    mIndices[3] = 1; mIndices[4] = 3; mIndices[5] = 2;
    // update gl index buffer
    resetIndexBuffer();

    // vertices
    mPositions[0].set(0.0f, 0.0f, 0.0f);
    mPositions[1].set(mSize.width(), 0.0f, 0.0f);
    mPositions[2].set(0.0f, mSize.height(), 0.0f);
    mPositions[3].set(mSize.width(), mSize.height(), 0.0f);
    for (int i = 0; i < 4; ++i) mTexCoords[i] = mPositions[i].vec2();

    // connections
    auto connects = mHexaConnections.data();
    for (int i = 0; i < 4; ++i) connects[i].clear();

    connects[0].id[0] = 1; connects[0].id[1] = 2;
    connects[1].id[0] = 0; connects[1].id[1] = 2; connects[1].id[2] = 3;
    connects[2].id[0] = 0; connects[2].id[1] = 1; connects[2].id[2] = 3;
    connects[3].id[0] = 1; connects[3].id[1] = 2;
}

void GridMesh::writeHeightMap(const HeightMap& aMap, const QVector2D& aMinPos)
{
    for (int i = 0; i < mVertexCount; ++i)
    {
        gl::Vector3 glpos = mPositions[i];

        // read height
        QVector3D c(glpos.x + aMinPos.x(), glpos.y + aMinPos.y(), 0.0f);
        c.setZ(aMap.readHeight(c.toVector2D()));
        mPositions[i].z = c.z();

        // read around height
        QVector3D l(c.x() - 2.0f, c.y(), 0.0f);
        QVector3D r(c.x() + 2.0f, c.y(), 0.0f);
        QVector3D u(c.x(), c.y() - 2.0f, 0.0f);
        QVector3D d(c.x(), c.y() + 2.0f, 0.0f);
        l.setZ(aMap.readHeight(l.toVector2D()));
        r.setZ(aMap.readHeight(r.toVector2D()));
        u.setZ(aMap.readHeight(u.toVector2D()));
        d.setZ(aMap.readHeight(d.toVector2D()));

        l = (l - c).normalized();
        r = (r - c).normalized();
        u = (u - c).normalized();
        d = (d - c).normalized();

        QVector3D normal;
        normal += QVector3D::crossProduct(u, l);
        normal += QVector3D::crossProduct(r, u);
        normal += QVector3D::crossProduct(l, d);
        normal += QVector3D::crossProduct(d, r);

        normal.setZ(-normal.z());
        mNormals[i].set((normal / 4).normalized());
    }
}

gl::BufferObject& GridMesh::getIndexBuffer()
{
    if (!mIndexBuffer)
    {
        mIndexBuffer.reset(new gl::BufferObject(GL_ELEMENT_ARRAY_BUFFER));
    }
    return *mIndexBuffer;
}

LayerMesh::MeshBuffer& GridMesh::getMeshBuffer()
{
    if (mMeshBuffer.isNull())
    {
        mMeshBuffer.reset(new MeshBuffer());
    }
    mMeshBuffer->reserve(mVertexCount);
    return *mMeshBuffer;
}

void GridMesh::resetArrayedConnection(
        ArrayedConnectionList& aDest, const gl::Vector3* aPositions) const
{
    ArrayedConnectionWriter writer(aDest, mVertexCount);

    for (int i = 0; i < mVertexCount; ++i)
    {
        writer.beginOneVertex(6);

        for (int dir = 0; dir < 6; ++dir)
        {
            if (mHexaConnections[i].has(dir))
            {
                const int connectIndex = mHexaConnections[i].id[dir];
                auto offset = aPositions[connectIndex] - mPositions[connectIndex];
                writer.pushPosition(offset.vec2());
            }
        }
    }
}

Frame GridMesh::frameSign() const
{
    return Frame(TimeLine::kDefaultKeyIndex);
}

util::ArrayBlock<gl::Vector3> GridMesh::createFFD(
        util::ArrayBlock<const gl::Vector3> aPrevFFD,
        const Transitions& aTrans) const
{
    XC_ASSERT(aTrans.data.count() > 0);
    XC_ASSERT(aTrans.data.count() == mVertexCount);

    if (mVertexCount == 0 || aTrans.data.count() == 0)
    {
        return util::ArrayBlock<gl::Vector3>(nullptr, 0); // fail safe code
    }

    auto count = aTrans.data.count();
    auto prev = aPrevFFD.array();
    auto prevCount = aPrevFFD.count();

    util::ArrayBlock<gl::Vector3> result(new gl::Vector3[count], count);
    QVector<bool> validity;
    std::vector<int> invalidIndices;
    std::vector<int> danglingIndices;
    validity.resize(count);
    invalidIndices.reserve(count);
    danglingIndices.reserve(std::max(1, (int)(qSqrt(count) * 4))); // a standard

    {
        int idx = 0;
        for (auto t : aTrans.data)
        {
            if (t.pos.isValid())
            {
                XC_ASSERT(t.id[0] < (GLuint)prevCount);
                XC_ASSERT(t.id[1] < (GLuint)prevCount);
                XC_ASSERT(t.id[2] < (GLuint)prevCount);
                const util::Triangle2D tri(
                            prev[t.id[0]].pos2D(),
                        prev[t.id[1]].pos2D(),
                        prev[t.id[2]].pos2D());
                auto pos = t.pos.get(tri) + aTrans.offset;
                result[idx] = gl::Vector3::make(pos.x(), pos.y(), 0.0f);
                validity[idx] = true;
            }
            else
            {
                result[idx] = mPositions[idx];
                validity[idx] = false;
                invalidIndices.push_back(idx);
            }
            ++idx;
        }
    }

#if 1
    while (!invalidIndices.empty())
    {
        for (auto itr = invalidIndices.begin(); itr != invalidIndices.end();)
        {
            const int idx = *itr;
            auto predict = gatherValidPositions(idx, result.array(), validity.data());
            if (predict.first)
            {
                result[idx] = predict.second;
                itr = invalidIndices.erase(itr);
                danglingIndices.push_back(idx);
            }
            else
            {
                ++itr;
            }
        }

        if (!danglingIndices.empty())
        {
            for (auto idx : danglingIndices)
            {
                validity[idx] = true;
            }
            danglingIndices.clear();
        }
        else
        {
            break;
        }
    }
#endif

    return result;
}

bool GridMesh::hasConnection(int aArrayIndex, int aIdIndex) const
{
    return mHexaConnections[aArrayIndex].has(aIdIndex);
}

int GridMesh::connectionId(int aArrayIndex, int aIdIndex) const
{
    return mHexaConnections[aArrayIndex].id[aIdIndex];
}


std::pair<bool, gl::Vector3> GridMesh::gatherValidPositions(
        int aIndex, const gl::Vector3* aPositions, const bool* aValidity) const
{
    const int connectionCount = kHexaConnectionCount;

    std::array<QVector2D, kMaxConnectionCount> result;
    int count = 0;

    for (int i = 0; i < connectionCount; ++i)
    {
        const int dir = i;
        if (hasConnection(aIndex, dir))
        {
            const int id = connectionId(aIndex, dir);
            XC_ASSERT(0 <= id && id < mVertexCount);

            if (!aValidity[id]) continue;

            auto pos = aPositions[id].pos2D();
            auto oriPos = mPositions[id].pos2D();
            auto oriVec = mPositions[aIndex].pos2D() - oriPos;

            std::array<QVector2D, kMaxConnectionCount> subResult;
            int subCount = 0;

            for (int k = 0; k < connectionCount; ++k)
            {
                const int subDir = k;
                if (hasConnection(id, subDir))
                {
                    const int id2 = connectionId(id, subDir);
                    XC_ASSERT(0 <= id2 && id2 < mVertexCount);

                    if (!aValidity[id2]) continue;

                    auto subPos = aPositions[id2].pos2D();
                    auto subOriPos = mPositions[id2].pos2D();
                    auto subVec = subPos - pos;
                    auto subOriVec = subOriPos - oriPos;
                    XC_ASSERT(!subVec.isNull());
                    XC_ASSERT(!subOriVec.isNull());

                    auto rotate = util::MathUtil::getAngleDifferenceRad(
                                subOriVec, subVec);

                    subResult[subCount] = pos +
                            util::MathUtil::getRotateVectorRad(oriVec, rotate);
                    ++subCount;
                }
            }

            XC_ASSERT(subCount <= kMaxConnectionCount);
            if (subCount > 0)
            {
                for (int m = 0; m < subCount; ++m)
                {
                    result[count] += subResult[m] / (float)subCount;
                }
            }
            else
            {
                result[count] = pos + oriVec;
            }
            ++count;
        }
    }

    // blend
    XC_ASSERT(count <= kMaxConnectionCount);
    QVector2D blended;
    for (int i = 0; i < count; ++i)
    {
        blended += result[i] / (float)count;
    }

    return std::pair<bool, gl::Vector3>(
                count > 0, gl::Vector3::make(blended.x(), blended.y(), 0.0f));
}

bool GridMesh::serialize(Serializer& aOut) const
{
    static const std::array<uint8, 8> kSignature =
        { 'G', 'r', 'i', 'd', 'M', 'e', 's', 'h' };

    // signature
    auto pos = aOut.beginBlock(kSignature);

    // type
    aOut.write(0);

    // info
    aOut.write(mSize);
    aOut.write(mOriginOffset);
    aOut.write(mCellPx);
    aOut.write(0); // primitive type

    // indices
    aOut.write(mIndexCount);
    if (mIndexCount > 0)
    {
        aOut.writeGL(mIndices.data(), mIndexCount);
    }

    // vertices
    aOut.write(mVertexCount);
    if (mVertexCount > 0)
    {
        aOut.writeGL(mPositions.data(), mVertexCount);
        aOut.writeGL(mOffsets.data(),   mVertexCount);
        aOut.writeGL(mTexCoords.data(), mVertexCount);
        aOut.writeGL(mNormals.data(),   mVertexCount);

        const size_t connectSize = mVertexCount * sizeof(HexaConnection);
        aOut.write(XCMemBlock((uint8*)mHexaConnections.data(), connectSize));
    }

    // vertex rect
    aOut.write(mVertexRect);

    aOut.endBlock(pos);

    return !aOut.failure();
}

bool GridMesh::deserialize(Deserializer& aIn)
{
    freeBuffers();

    // check block begin
    if (!aIn.beginBlock("GridMesh"))
    {
        return false;
    }
    aIn.pushLogScope("GridMesh");

    // type
    int type = 0;
    aIn.read(type);
    if (type != 0) return aIn.errored("invalid type");

    // info
    aIn.read(mSize);
    aIn.read(mOriginOffset);
    aIn.read(mCellPx);
    // primitive
    {
        int primType = 0;
        aIn.read(primType);
        if (primType != 0) return aIn.errored("invalid primitive type");
    }

    // index count
    aIn.read(mIndexCount);
    if (mIndexCount < 0) return aIn.errored("invalid index count");

    // indices
    if (mIndexCount > 0)
    {
        allocIndices(mIndexCount);
        aIn.readGL(mIndices.data(), mIndexCount);
    }
    // initialize index buffer
    resetIndexBuffer();

    // vertex count
    aIn.read(mVertexCount);
    if (mVertexCount < 0) return aIn.errored("invalid vertex count");

    // vertices
    if (mVertexCount > 0)
    {
        allocVertexBuffers(mVertexCount);
        aIn.readGL(mPositions.data(), mVertexCount);
        aIn.readGL(mOffsets.data(),   mVertexCount);
        aIn.readGL(mTexCoords.data(), mVertexCount);
        aIn.readGL(mNormals.data(),   mVertexCount);

        const size_t connectSize = mVertexCount * sizeof(HexaConnection);
        if (!aIn.read(XCMemBlock((uint8*)mHexaConnections.data(), connectSize)))
        {
            return aIn.errored("failed to read connections");
        }
    }

    // vertex rect
    if (aIn.version() >= QVersionNumber(0, 5))
    {
        aIn.read(mVertexRect);
    }
    else
    {
        // note: the bug on version 0.4 or lower. mVertexRect doesn't be serialized.
        auto positions = mPositions.data();
        float l = 0.0f, t = 0.0f, r = 0.0f, b = 0.0f;
        for (int i = 0; i < mVertexCount; ++i)
        {
            auto pos = positions[i];
            l = std::min(l, pos.x - 1);
            t = std::min(t, pos.y - 1);
            r = std::max(r, pos.x + 1);
            b = std::max(b, pos.y + 1);
        }
        mVertexRect = QRect(l, t, r - l, b - t);
    }

    // initialize mesh buffer
    getMeshBuffer();

    // check block end
    if (!aIn.endBlock())
    {
        return false;
    }
    aIn.popLogScope();

    return !aIn.failure();
}

//-------------------------------------------------------------------------------------------------
GridMesh::TransitionCreater::TransitionCreater(const GridMesh& aPrev, const QPoint& aTopLeft)
    : mRect(aPrev.vertexRect())
    , mIndexCount(aPrev.indexCount())
    , mIndices()
    , mPositions()
    , mTopLeft(aTopLeft)
{
    XC_ASSERT(aPrev.primitiveMode() == GL_TRIANGLES);
    if (mIndexCount > 0)
    {
        mIndices.reset(new GLuint[mIndexCount]);
        memcpy(mIndices.data(), aPrev.indices(), sizeof(GLuint) * mIndexCount);
    }
    const int vertexCount = aPrev.vertexCount();
    if (vertexCount > 0)
    {
        mPositions.reset(new gl::Vector3[vertexCount]);
        memcpy(mPositions.data(), aPrev.positions(), sizeof(gl::Vector3) * vertexCount);
    }
}

GridMesh::Transitions GridMesh::TransitionCreater::create(
        const gl::Vector3* aNext, int aCount, const QPoint& aTopLeft)
{
    if (mIndexCount == 0 || mPositions.isNull() || !aNext || aCount == 0)
    {
        return Transitions();
    }

    XC_ASSERT(mIndices);

    const QRectF space(mRect);
    util::BinarySpacePartition2D<TriId> bsp(space);

    for (int i = 0; i < mIndexCount; i += 3)
    {
        TriId a = { mIndices[i], mIndices[i + 1], mIndices[i + 2] };
        util::Triangle2D ta(
                    mPositions[a[0]].pos2D(),
                mPositions[a[1]].pos2D(),
                mPositions[a[2]].pos2D());

        bool result = false;

        if (ta.hasFace(FLT_MIN))
        {
            result = bsp.push(a, ta);
            XC_ASSERT(result);
        }
    }

    const QVector2D offset(aTopLeft - mTopLeft);
    auto positions = aNext;
    auto count = aCount;

    Transitions result;
    result.data.resize(count);
    result.offset = -offset;

    for (int i = 0; i < count; ++i)
    {
        auto glpos = positions[i];
        auto pos = glpos.pos2D() + offset;
        auto obj = bsp.findOne(pos.toPointF());

        if (obj)
        {
            Transition trans;
            trans.id = obj->data;
            trans.pos = util::Triangle2DPos::make(obj->tri, pos);
            result.data[i] = trans;
        }
    }

    return result;
}

} // namespace core
