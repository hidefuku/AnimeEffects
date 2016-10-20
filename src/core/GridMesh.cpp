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

namespace core
{

//-------------------------------------------------------------------------------------------------
void GridMesh::QuadConnection::clear()
{
    for (int i = 0; i < util::Dir4_TERM; ++i)
    {
        id[i] = -1;
    }
}

//-------------------------------------------------------------------------------------------------
GridMesh::GridMesh()
    : mSize()
    , mCellNumX(0)
    , mCellNumY(0)
    , mCellPx(0)
    , mPrimitiveType(GL_TRIANGLES)
    , mIndexCount(0)
    , mVertexCount(0)
    , mVertexRect()
    , mIndices()
    , mPositions()
    , mOffsets()
    , mTexCoords()
    , mNormals()
    , mQuadConnections()
    , mHexaConnections()
    , mMeshBuffer()
{
    // initialize mesh buffer
    getMeshBuffer();
}

GridMesh::~GridMesh()
{
    freeBuffers();
}

void GridMesh::freeBuffers()
{
    mPositions.reset();
    mOffsets.reset();
    mTexCoords.reset();
    mNormals.reset();
    mHexaConnections.reset();
    mQuadConnections.reset();
    mIndices.reset();
}

void GridMesh::allocVertexBuffers(int aVertexCount)
{
    mPositions.reset(new gl::Vector3[aVertexCount]);
    mOffsets.reset(new gl::Vector3[aVertexCount]);
    mTexCoords.reset(new gl::Vector2[aVertexCount]);
    mNormals.reset(new gl::Vector3[aVertexCount]);

    if (mPrimitiveType == GL_TRIANGLES)
    {
        mHexaConnections.reset(new HexaConnection[aVertexCount]);
    }
    else
    {
        mQuadConnections.reset(new QuadConnection[aVertexCount]);
    }
}

void GridMesh::allocIndexBuffer(int aIndexCount)
{
    mIndices.reset();
    mIndices.reset(new GLuint[aIndexCount]);
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
    //createFromImageVer1(aImagePtr, aSize, aCellPx);
    createFromImageVer2(aImagePtr, aSize, aCellPx);
}

void GridMesh::createFromImageVer2(const void* aImagePtr, const QSize& aSize, int aCellPx)
{
    XC_PTR_ASSERT(aImagePtr);
    XC_ASSERT(aSize.width() > 0 && aSize.height() > 0);
    XC_ASSERT(aCellPx > 0);

    // free old buffers
    freeBuffers();

    mSize = aSize;
    mCellPx = aCellPx;
    mPrimitiveType = GL_TRIANGLES;

    img::GridMeshCreator creator((const uint8*)aImagePtr, aSize, aCellPx);

    mVertexRect = creator.vertexRect();
    mVertexCount = creator.vertexCount();
    mIndexCount = creator.indexCount();

    // return if any vertices does not exist
    if (mVertexCount <= 0 || mIndexCount <= 0) return;

    // allocate attribute buffers
    allocIndexBuffer(mIndexCount);
    allocVertexBuffers(mVertexCount);
    initializeVertexBuffers(mVertexCount);

    // setup attributes
    creator.writeIndices(mIndices.data());
    creator.writeVertices((GLfloat*)mPositions.data(), (GLfloat*)mTexCoords.data());

    // setup connections
    creator.writeConnections(mHexaConnections.data());
}

#if 1
void GridMesh::createFromImageVer1(const void* aImagePtr, const QSize& aSize, int aCellPx)
{
    using img::PixelPos;
    using img::Quad;

    struct Cell
    {
        bool isExist;
        bool hasXlu;
        Quad quad;

        inline void reset()
        {
            isExist = false;
            hasXlu = false;
            quad.reset();
        }
    };

    class CellSet
    {
    public:
        CellSet(int aCellNumX, int aCellNumY, int aPixel)
            : mCells()
            , mPixel(aPixel)
            , mNumX(aCellNumX)
            , mNumY(aCellNumY)
        {
            // allocate cells
            mCells.reset(new Cell[mNumX * mNumY]);
            XC_PTR_ASSERT(mCells.data());

            // initialize cell exist flags
            for (int y = 0; y < mNumY; ++y)
            {
                int indexY = y * mNumX;
                for (int x = 0; x < mNumX; ++x)
                {
                    mCells[indexY + x].reset();
                }
            }
        }

        // calculate cells from a image
        void readImage(const void* aImagePtr, int aImageWidth, int aImageHeight)
        {
            static const uint8 kAlphaThreshold = 3;
            const uint8* alphaPtr = ((const uint8*)aImagePtr) + 3;

            for (int pixelY = 0; pixelY < aImageHeight; ++pixelY)
            {
                const int cellY = std::min(pixelY / mPixel, mNumY - 1) * mNumX;
                for (int pixelX = 0; pixelX < aImageWidth; ++pixelX)
                {
                    const int cellX = std::min(pixelX / mPixel, mNumX - 1);
                    auto& cell = mCells[cellY + cellX];

                    if (*alphaPtr >= kAlphaThreshold)
                    {
                        cell.isExist = true;
                        cell.quad.extend(pixelX, pixelY);
                    }
                    else
                    {
                        cell.hasXlu = true;
                    }
                    alphaPtr += 4;
                }
            }

            connectQuads();

            makeSureConvexQuads();
        }

        int makeVertexIds()
        {
            int count = 0;
            for (int y = 0; y < mNumY; ++y)
            {
                for (int x = 0; x < mNumX; ++x)
                {
                    Cell* self = existCellPtr(x, y);
                    if (self)
                    {
                        Cell* lu = existCellPtr(x - 1, y - 1);
                        Cell* up = existCellPtr(x, y - 1);
                        Cell* ru = existCellPtr(x + 1, y - 1);
                        Cell* ll = existCellPtr(x - 1, y);

                        // left top vertex
                        if ((!lu || !self->quad.lt().tryMergeId(lu->quad.rb())) &&
                            (!up || !self->quad.lt().tryMergeId(up->quad.lb())) &&
                            (!ll || !self->quad.lt().tryMergeId(ll->quad.rt())))
                        {
                            self->quad.lt().id = count;
                            ++count;
                        }

                        // right top vertex
                        if ((!up || !self->quad.rt().tryMergeId(up->quad.rb())) &&
                            (!ru || !self->quad.rt().tryMergeId(ru->quad.lb())))
                        {
                            self->quad.rt().id = count;
                            ++count;
                        }

                        // left bottom vertex
                        if ((!ll || !self->quad.lb().tryMergeId(ll->quad.rb())))
                        {
                            self->quad.lb().id = count;
                            ++count;
                        }

                        // right bottom vertex
                        self->quad.rb().id = count;
                        ++count;
                    }
                }
            }
            return count;
        }

        GLsizei calculateIndexCount() const
        {
            GLsizei count = 0;
            for (int y = 0; y < mNumY; ++y)
            {
                for (int x = 0; x < mNumX; ++x)
                {
                    if (cell(x, y).isExist) count += 4;
                }
            }
            return count;
        }

        inline bool isIn(int aX, int aY) const
        {
            return (0 <= aX && aX < mNumX) && (0 <= aY && aY < mNumY);
        }

        inline Cell* cellPtr(int aX, int aY)
        {
            if (!isIn(aX, aY)) return NULL;
            return &(mCells[mNumX * aY + aX]);
        }

        inline Cell* existCellPtr(int aX, int aY)
        {
            Cell* p = cellPtr(aX, aY);
            return (p && !p->isExist) ? NULL : p;
        }

        inline Cell& cell(int aX, int aY)
        {
            return mCells[mNumX * aY + aX];
        }

        inline const Cell& cell(int aX, int aY) const
        {
            return mCells[mNumX * aY + aX];
        }

        inline int numX() const { return mNumX; }

        inline int numY() const { return mNumY; }

        inline int pixel() const { return mPixel; }

    private:
        void connectQuads()
        {
            for (int y = 0; y < mNumY; ++y)
            {
                for (int x = 0; x < mNumX; ++x)
                {
                    Cell* self = existCellPtr(x, y);
                    if (self)
                    {
                        Cell* right = existCellPtr(x + 1, y);
                        if (right) tryConnectLeftRight(self, right);

                        Cell* down = existCellPtr(x, y + 1);
                        if (down) tryConnectUpDown(self, down);
                    }
                }
            }
        }

        void tryConnectLeftRight(Cell* aLeft, Cell* aRight)
        {
            if (aLeft->quad.rt().x == aRight->quad.lt().x &&
                aLeft->quad.rb().x == aRight->quad.lb().x)
            {
                if (aLeft->quad.rt().y > aRight->quad.lt().y)
                {
                    aLeft->quad.rt().y = aRight->quad.lt().y;
                }
                else
                {
                    aRight->quad.lt().y = aLeft->quad.rt().y;
                }
                if (aLeft->quad.rb().y < aRight->quad.lb().y)
                {
                    aLeft->quad.rb().y = aRight->quad.lb().y;
                }
                else
                {
                    aRight->quad.lb().y = aLeft->quad.rb().y;
                }
            }
        }

        void tryConnectUpDown(Cell* aUp, Cell* aDown)
        {
            if (aUp->quad.lb().y == aDown->quad.lt().y &&
                aUp->quad.rb().y == aDown->quad.rt().y)
            {
                if (aUp->quad.lb().x > aDown->quad.lt().x)
                {
                    aUp->quad.lb().x = aDown->quad.lt().x;
                }
                else
                {
                    aDown->quad.lt().x = aUp->quad.lb().x;
                }
                if (aUp->quad.rb().x < aDown->quad.rt().x)
                {
                    aUp->quad.rb().x = aDown->quad.rt().x;
                }
                else
                {
                    aDown->quad.rt().x = aUp->quad.rb().x;
                }
            }
        }

        void makeSureConvexQuads()
        {
            for (int y = 0; y < mNumY; ++y)
            {
                for (int x = 0; x < mNumX; ++x)
                {
                    if (cell(x, y).isExist) cell(x, y).quad.makeSureConvex();
                }
            }
        }

        QScopedArrayPointer<Cell> mCells;
        int mPixel;
        int mNumX;
        int mNumY;
    };

    XC_PTR_ASSERT(aImagePtr);
    XC_ASSERT(aSize.width() > 0 && aSize.height() > 0);
    XC_ASSERT(aCellPx > 0);
    const int width = aSize.width();
    const int height = aSize.height();

    // free old buffers
    freeBuffers();

    mSize = aSize;
    mPrimitiveType = GL_QUADS;
    mVertexRect = QRect(QPoint(0, 0), aSize);

    // calculate cell information
    {
        mCellPx = aCellPx;
        mCellNumX = (width / mCellPx) + 1;
        mCellNumY = (height / mCellPx) + 1;
    }

    // initialize cell set
    CellSet cellSet(mCellNumX, mCellNumY, mCellPx);

    // read a image
    cellSet.readImage(aImagePtr, width, height);

    // make the vertex structure
    mVertexCount = cellSet.makeVertexIds();
    mIndexCount = (int)cellSet.calculateIndexCount();

    // return if any vertices does not exist
    if (mVertexCount <= 0 || mIndexCount <= 0) return;

    // allocate attribute buffers
    {
        allocIndexBuffer(mIndexCount);
        allocVertexBuffers(mVertexCount);
        initializeVertexBuffers(mVertexCount);
    }

    // setup attributes
    {
        GLsizei index = 0;
        for (int y = 0; y < cellSet.numY(); ++y)
        {
            for (int x = 0; x < cellSet.numX(); ++x)
            {
                Cell* cell = cellSet.existCellPtr(x, y);
                if (cell)
                {
                    for (int i = 0; i < 4; ++i)
                    {
                        PixelPos pixelPos = cell->quad.pos[i];
                        mPositions[pixelPos.id].set(pixelPos.x, pixelPos.y, 0.0f);
                        mTexCoords[pixelPos.id].set((float)pixelPos.x / width, (float)pixelPos.y / height);
                    }
                    mIndices[index] = cell->quad.lt().id; ++index;
                    mIndices[index] = cell->quad.rt().id; ++index;
                    mIndices[index] = cell->quad.rb().id; ++index;
                    mIndices[index] = cell->quad.lb().id; ++index;
                }
            }
        }
    }

    // setup connections
    {
        for (int i = 0; i < mVertexCount; ++i)
        {
            mQuadConnections[i].clear();
        }

        for (int y = 0; y < cellSet.numY(); ++y)
        {
            for (int x = 0; x < cellSet.numX(); ++x)
            {
                Cell* cell = cellSet.existCellPtr(x, y);

                if (cell)
                {
                    auto lt = cell->quad.lt();
                    auto rt = cell->quad.rt();
                    auto rb = cell->quad.rb();
                    auto lb = cell->quad.lb();
                    Cell* u = cellSet.existCellPtr(x, y - 1);
                    Cell* l = cellSet.existCellPtr(x - 1, y);
                    Cell* d = cellSet.existCellPtr(x, y + 1);
                    Cell* r = cellSet.existCellPtr(x + 1, y);

                    // lt
                    {
                        auto id = lt.id;
                        auto& connect = mQuadConnections[id];
                        if (id != lb.id) connect.setDown(lb.id);
                        if (id != rt.id) connect.setRight(rt.id);

                        if (u && id == u->quad.lb().id && u->quad.hasLRange())
                            connect.setUp(u->quad.lt().id);

                        if (l && id == l->quad.rt().id && l->quad.hasTRange())
                            connect.setLeft(l->quad.lt().id);
                    }
                    // rt
                    {
                        auto id = rt.id;
                        auto& connect = mQuadConnections[id];
                        if (id != rb.id) connect.setDown(rb.id);
                        if (id != lt.id) connect.setLeft(lt.id);

                        if (u && id == u->quad.rb().id && u->quad.hasRRange())
                            connect.setUp(u->quad.rt().id);

                        if (r && id == r->quad.lt().id && r->quad.hasTRange())
                            connect.setRight(r->quad.rt().id);
                    }
                    // lb
                    {
                        auto id = lb.id;
                        auto& connect = mQuadConnections[id];
                        if (id != lt.id) connect.setUp(lt.id);
                        if (id != rb.id) connect.setRight(rb.id);

                        if (d && id == d->quad.lt().id && d->quad.hasLRange())
                            connect.setDown(d->quad.lb().id);

                        if (l && id == l->quad.rb().id && l->quad.hasBRange())
                            connect.setLeft(l->quad.lb().id);
                    }
                    // rb
                    {
                        auto id = rb.id;
                        auto& connect = mQuadConnections[id];
                        if (id != rt.id) connect.setUp(rt.id);
                        if (id != lb.id) connect.setLeft(lb.id);

                        if (d && id == d->quad.rt().id && d->quad.hasRRange())
                            connect.setDown(d->quad.rb().id);

                        if (r && id == r->quad.lb().id && r->quad.hasBRange())
                            connect.setRight(r->quad.rb().id);
                    }
                }
            }
        }

    }

    //qDebug() << "test end";
}
#endif

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
    if (mPrimitiveType == GL_TRIANGLES)
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
    else
    {
        ArrayedConnectionWriter writer(aDest, mVertexCount);

        for (int i = 0; i < mVertexCount; ++i)
        {
            writer.beginOneVertex(4);

            for (int di = 0; di < util::Dir4_TERM; ++di)
            {
                auto dir = (util::Dir4)di;
                if (mQuadConnections[i].has(dir))
                {
                    const int connectIndex = mQuadConnections[i].id[dir];
                    auto offset = aPositions[connectIndex] - mPositions[connectIndex];
                    writer.pushPosition(offset.vec2());
                }
            }
        }
    }
}

util::ArrayBlock<gl::Vector3> GridMesh::createFFD(
        util::ArrayBlock<const gl::Vector3> aPrevFFD,
        const Transitions& aTrans) const
{
    XC_ASSERT(aTrans.data.count() > 0);
    XC_ASSERT(aTrans.data.count() == mVertexCount);
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
    return (mPrimitiveType == GL_TRIANGLES) ?
                mHexaConnections[aArrayIndex].has(aIdIndex) :
                mQuadConnections[aArrayIndex].has(aIdIndex);
}

int GridMesh::connectionId(int aArrayIndex, int aIdIndex) const
{
    return (mPrimitiveType == GL_TRIANGLES) ?
                mHexaConnections[aArrayIndex].id[aIdIndex] :
                mQuadConnections[aArrayIndex].id[aIdIndex];
}


std::pair<bool, gl::Vector3> GridMesh::gatherValidPositions(
        int aIndex, const gl::Vector3* aPositions, const bool* aValidity) const
{
    const int connectionCount =
            (mPrimitiveType == GL_TRIANGLES) ?
                kHexaConnectionCount : kQuadConnectionCount;

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
    aOut.write(mCellNumX);
    aOut.write(mCellNumY);
    aOut.write(mCellPx);
    aOut.write((mPrimitiveType == GL_TRIANGLES) ? 0 : 1);

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

        if (mPrimitiveType == GL_TRIANGLES)
        {
            const size_t connectSize = mVertexCount * sizeof(HexaConnection);
            aOut.write(XCMemBlock((uint8*)mHexaConnections.data(), connectSize));
        }
        else
        {
            const size_t connectSize = mVertexCount * sizeof(QuadConnection);
            aOut.write(XCMemBlock((uint8*)mQuadConnections.data(), connectSize));
        }
    }

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
    if (type != 0) return false;

    // info
    aIn.read(mCellNumX);
    aIn.read(mCellNumY);
    aIn.read(mCellPx);
    // primitive
    {
        int prim = 0;
        aIn.read(prim);
        mPrimitiveType = (prim == 0) ? GL_TRIANGLES : GL_QUADS;
    }

    // index count
    aIn.read(mIndexCount);
    if (mIndexCount < 0) return false;

    // indices
    if (mIndexCount > 0)
    {
        allocIndexBuffer(mIndexCount);
        aIn.readGL(mIndices.data(), mIndexCount);
    }

    // vertex count
    aIn.read(mVertexCount);
    if (mVertexCount < 0) return false;

    // vertices
    if (mVertexCount > 0)
    {
        allocVertexBuffers(mVertexCount);
        aIn.readGL(mPositions.data(), mVertexCount);
        aIn.readGL(mOffsets.data(),   mVertexCount);
        aIn.readGL(mTexCoords.data(), mVertexCount);
        aIn.readGL(mNormals.data(),   mVertexCount);

        if (mPrimitiveType == GL_TRIANGLES)
        {
            const size_t connectSize = mVertexCount * sizeof(HexaConnection);
            if (!aIn.read(XCMemBlock((uint8*)mHexaConnections.data(), connectSize)))
            {
                return aIn.errored("failed to read connections");
            }
        }
        else
        {
            const size_t connectSize = mVertexCount * sizeof(QuadConnection);
            if (!aIn.read(XCMemBlock((uint8*)mQuadConnections.data(), connectSize)))
            {
                return aIn.errored("failed to read connections");
            }
        }
    }

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
    , mQuad(aPrev.primitiveMode() == GL_QUADS)
{
    XC_ASSERT(aPrev.primitiveMode() == GL_TRIANGLES ||
              aPrev.primitiveMode() == GL_QUADS);
    const int vertexCount = aPrev.vertexCount();
    mIndices.reset(new GLuint[mIndexCount]);
    mPositions.reset(new gl::Vector3[vertexCount]);
    memcpy(mIndices.data(), aPrev.indices(), sizeof(GLuint) * mIndexCount);
    memcpy(mPositions.data(), aPrev.positions(), sizeof(gl::Vector3) * vertexCount);
}

GridMesh::Transitions GridMesh::TransitionCreater::create(
        const gl::Vector3* aNext, int aCount, const QPoint& aTopLeft)
{
    XC_ASSERT(mIndices);

    const QRectF space(mRect);
    util::BinarySpacePartition2D<TriId> bsp(space);

    if (mQuad)
    {
        for (int i = 0; i < mIndexCount; i += 4)
        {
            TriId a = { mIndices[i], mIndices[i + 1], mIndices[i + 2] };
            TriId b = { mIndices[i], mIndices[i + 2], mIndices[i + 3] };
            util::Triangle2D ta(
                    mPositions[a[0]].pos2D(),
                    mPositions[a[1]].pos2D(),
                    mPositions[a[2]].pos2D());
            util::Triangle2D tb(
                    mPositions[b[0]].pos2D(),
                    mPositions[b[1]].pos2D(),
                    mPositions[b[2]].pos2D());

            bool result = false;

            if (ta.hasFace(FLT_MIN))
            {
                result = bsp.push(a, ta);
                XC_ASSERT(result);
            }
            if (tb.hasFace(FLT_MIN))
            {
                result = bsp.push(b, tb);
                XC_ASSERT(result);
            }
        }
    }
    else
    {
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
