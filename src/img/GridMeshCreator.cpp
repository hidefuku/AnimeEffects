#include "util/MathUtil.h"
#include "util/TriangleRasterizer.h"
#include "img/GridMeshCreator.h"
#include "img/Util.h"
#include "img/ColorRGBA.h"

namespace img
{

//-------------------------------------------------------------------------------------------------
void GridMeshCreator::HexaConnection::clear()
{
    for (int i = 0; i < 6; ++i)
    {
        id[i] = -1;
    }
}

//-------------------------------------------------------------------------------------------------
GridMeshCreator::Image::Image(const uint8* aPtr, const QSize& aSize)
    : mBuffer()
    , mData()
    , mSize(aSize)
{
    XC_PTR_ASSERT(aPtr);
    XC_ASSERT(!aSize.isEmpty());

    mBuffer.alloc(Format_RGBA8, mSize);
    memcpy(mBuffer.data(), aPtr, mBuffer.size());
    img::Util::expandAlpha1Pixel(mBuffer.data(), mSize);
    mData = mBuffer.data();
}

bool GridMeshCreator::Image::hasSomeAlphaIn3x3(int aX, int aY) const
{
    for (int i = -1; i < 2; ++i)
    {
        for (int k = -1; k < 2; ++k)
        {
            if (hasAlpha(aX + i, aY + k)) return true;
        }
    }
    return false;
}

bool GridMeshCreator::Image::getOpaExistence(const Cell& aCell, const QSizeF& aCellSize) const
{
    const int t = (int)(aCell.y);
    const int b = (int)(aCell.y + aCellSize.height());
    const int h = b - t;
    //const float h = aCellSize.height();

    const float cellWidth = aCellSize.width();
    const float halfCellWidth = cellWidth * 0.5f;

    for (int y = t; y <= b; ++y)
    {
        if (y < 0 || mSize.height() <= y) continue;

        const float offs =
                aCell.inverted ?
                    halfCellWidth * (float)(y - t) / h :
                    halfCellWidth * (float)(h - y + t) / h;

        const int l = (int)(aCell.x + offs);
        const int r = (int)(aCell.x + cellWidth - offs);

        for (int x = l; x <= r; ++x)
        {
            if (x < 0 || mSize.width() <= x) continue;

            if (hasRawAlpha(x, y))
            {
                return true;
            }
        }
    }
    return false;
}

#if 0
void GridMeshCreator::Image::expandUseBitsEdgeOnePixel()
{
    const int uw = mUseBitsSize.width();
    const int uh = mUseBitsSize.height();
    uint8* bits = mUseBits.data();

    // horizontal
    for (int y = 0; y < uh; ++y)
    {
        uint8* line = bits + y * uw;
        uint8* prev = line;

        for (int x = 1; x < uw; ++x)
        {
            uint8* curr = line + x;
            if (*curr == 0)
            {
                if (*prev > 0) *curr = 1;
            }
            else
            {
                if (*prev == 0) *prev = 1;
            }
            prev = curr;
        }
    }

    // vertical
    for (int x = 0; x < uw; ++x)
    {
        uint8* line = bits + x;
        uint8* prev = line;

        for (int y = 1; y < uh; ++y)
        {
            uint8* curr = line + y * uw;
            if (*curr == 0)
            {
                if (*prev > 0) *curr = 1;
            }
            else
            {
                if (*prev == 0) *prev = 1;
            }
            prev = curr;
        }
    }
}
#endif

//-------------------------------------------------------------------------------------------------
GridMeshCreator::VertexTable::VertexTable(int aWidth, int aHeight)
    : mVertices()
    , mSize(aWidth, aHeight)
    , mHalfSqrt3((float)(std::sqrt(3.0) * 0.5))
{
    const int count = mSize.width() * mSize.height();
    mVertices.reset(new Vertex[count]);

    for (int i = 0; i < count; ++i)
    {
        auto& vertex = mVertices[i];
        vertex.isExist = false;
        vertex.x = 0.0f;
        vertex.y = 0.0f;
        vertex.index = 0;
        for (int i = 0; i < 6; ++i)
        {
            vertex.cell[i] = -1;
        }
        vertex.maxReduce = 0.0f;
        vertex.reduceRate = 0.0f;
    }
}

void GridMeshCreator::VertexTable::setReducingVectors(float aCellWidth)
{
    const float cellHeight = aCellWidth * mHalfSqrt3;
    const float pairTriangleHeight = cellHeight * mHalfSqrt3;
    QVector2D vecs[6];
    for (int i = 0; i < 6; ++i)
    {
        const float angle = (float)(M_PI / 6.0f + i * M_PI / 3.0f);
        vecs[i] = util::MathUtil::getVectorFromPolarCoord(cellHeight, angle);
    }

    const int count = mSize.width() * mSize.height();

    for (int i = 0; i < count; ++i)
    {
        auto& vtx = mVertices[i];
        QVector2D vec;
        int count = 0;

        for (int k = 0; k < 6; ++k)
        {
            if (vtx.cell[k] != -1)
            {
                vec += vecs[k];
                ++count;
            }
        }
        if (count > 0)
        {
            vtx.reduceVec = vec / (float)count;

            const float veclen = vtx.reduceVec.length();
            if (count == 2 && veclen >= pairTriangleHeight * 0.95f)
            {
                vtx.reduceVec *= aCellWidth / veclen;
            }
            else if (count == 3 && veclen >= aCellWidth * 0.57f)
            {
                vtx.reduceVec *= cellHeight / veclen;
            }
            else if (count == 4 && veclen >= pairTriangleHeight * 0.45f)
            {
                vtx.reduceVec *= aCellWidth / veclen;
            }

            vtx.maxReduce = vtx.reduceVec.length();

            if (vtx.maxReduce >= 1.0f) // 1 pixel or over
            {
                vtx.reduceVec /= vtx.maxReduce;
            }
            else
            {
                vtx.reduceVec = QVector2D();
                vtx.maxReduce = 0.0f;
            }
        }
    }
}

void GridMeshCreator::VertexTable::shortenReducingVectorsOnePixel()
{
    static const float kShorten = 1.0f;
    const int count = mSize.width() * mSize.height();

    for (int i = 0; i < count; ++i)
    {
        auto& vtx = mVertices[i];

        const float reduceLength = vtx.maxReduce * vtx.reduceRate;
        if (reduceLength >= kShorten)
        {
            vtx.reduceRate -= (kShorten / vtx.maxReduce);
            if (vtx.reduceRate < 0.0f) vtx.reduceRate = 0.0f;
        }
    }
}

//-------------------------------------------------------------------------------------------------
QSizeF GridMeshCreator::CellTable::calculateCellSize(int aCellWidth)
{
    return QSizeF(aCellWidth, (float)(aCellWidth * std::sqrt(3.0) * 0.5));
}

QSize GridMeshCreator::CellTable::calculateCellTableSize(
        const QSize& aImageSize, const QSizeF& aCellSize)
{
    const float halfCellWidth = aCellSize.width() * 0.5f;
    auto width = (int)((float)aImageSize.width() / halfCellWidth + 2);
    auto height = (int)((float)aImageSize.height() / aCellSize.height() + 1);
    return QSize(width, height);
}

GridMeshCreator::CellTable::CellTable(int aCellWidth)
    : mCells()
    , mCellSize(calculateCellSize(aCellWidth))
    , mWidth()
    , mHeight()
{
}

int GridMeshCreator::CellTable::initCells(const Image &aImage)
{
    int count = 0;

    const float cellWidth = mCellSize.width();
    const float cellHeight = mCellSize.height();
    const float halfCellWidth = cellWidth * 0.5f;
    auto tableSize = calculateCellTableSize(aImage.size(), mCellSize);
    mWidth = tableSize.width();
    mHeight = tableSize.height();
    mCells.reset(new Cell[mWidth * mHeight]);

    // initialize each cells
    for (int y = 0; y < mHeight; ++y)
    {
        const bool zalign = (y % 2 == 0);
        const int line = y * mWidth;
        for (int x = 0; x < mWidth; ++x)
        {
            auto& cell = mCells[line + x];

            // initilize parameters of a cell
            cell.nonReducing = false;
            cell.reducingFixed = false;
            cell.inverted = zalign ^ (x % 2 == 1);
            cell.x = halfCellWidth * (x - 1);
            cell.y = cellHeight * y;
            for (int i = 0; i < 3; ++i)
            {
                cell.vtx[i] = nullptr;
            }

            // set the existence of the cell by reading alpha bits
            cell.isExist = aImage.getOpaExistence(cell, mCellSize);

            if (cell.isExist)
            {
                ++count;
            }
        }
    }
    return count;
}

void GridMeshCreator::CellTable::connectCellsToVertices(VertexTable& aTable)
{
    for (int y = 0; y < mHeight; ++y)
    {
        const bool zalign = (y % 2 == 0);

        const int line = y * mWidth;
        for (int x = 0; x < mWidth; ++x)
        {
            const int index = line + x;
            auto& cell = mCells[index];

            // find vertices corresponded with a cell from VertexTable
            if (cell.isExist)
            {
                Vertex* v[3] = {};

                if (cell.inverted)
                {
                    if (zalign)
                    {
                        const int hx = x / 2;
                        v[0] = &(aTable.vertex(hx, y));
                        v[1] = &(aTable.vertex(hx + 1, y));
                        v[2] = &(aTable.vertex(hx, y + 1));
                    }
                    else
                    {
                        const int hx = x / 2;
                        v[0] = &(aTable.vertex(hx, y));
                        v[1] = &(aTable.vertex(hx + 1, y));
                        v[2] = &(aTable.vertex(hx + 1, y + 1));
                    }
                    v[0]->cell[0] = index;
                    v[1]->cell[2] = index;
                    v[2]->cell[4] = index;
                }
                else
                {
                    if (zalign)
                    {
                        const int hx = (x + 1) / 2;
                        v[0] = &(aTable.vertex(hx, y));
                        v[1] = &(aTable.vertex(hx, y + 1));
                        v[2] = &(aTable.vertex(hx - 1, y + 1));
                    }
                    else
                    {
                        const int hx = x / 2;
                        v[0] = &(aTable.vertex(hx, y));
                        v[1] = &(aTable.vertex(hx + 1, y + 1));
                        v[2] = &(aTable.vertex(hx, y + 1));
                    }
                    v[0]->cell[1] = index;
                    v[1]->cell[3] = index;
                    v[2]->cell[5] = index;
                }

                for (int i = 0; i < 3; ++i)
                {
                    v[i]->isExist = true;
                    cell.vtx[i] = v[i];
                }
            }
        }
    }
}

GridMeshCreator::Cell& GridMeshCreator::CellTable::cell(int aX, int aY)
{
    XC_ASSERT(0 <= aX && aX < mWidth && 0 <= aY && aY < mHeight);
    return mCells[aX + mWidth * aY];
}

GridMeshCreator::Cell* GridMeshCreator::CellTable::findExistingCell(int aX, int aY)
{
    if (aX < 0 || mWidth <= aX || aY < 0 || mHeight <= aY) return nullptr;
    Cell* cell = &(mCells[aX + mWidth * aY]);
    if (!cell->isExist) return nullptr;
    return cell;
}

//-------------------------------------------------------------------------------------------------
int GridMeshCreator::getCellTableCount(const QSize& aImageSize, int aCellWidth)
{
    auto cellSize = CellTable::calculateCellSize(aCellWidth);
    auto tableSize = CellTable::calculateCellTableSize(aImageSize, cellSize);
    return tableSize.width() * tableSize.height();
}

//-------------------------------------------------------------------------------------------------
GridMeshCreator::GridMeshCreator(const uint8* aPtr, const QSize& aSize, int aCellPx)
    : mCells()
    , mVertices()
    , mVertexCount()
    , mIndexCount()
    , mImageSize(aSize)
{
    execute(aPtr, aSize, aCellPx);
}

QRect GridMeshCreator::vertexRect() const
{
    const float halfwidth = mCells->cellWidth() * 0.5f;
    return QRect(
                QPoint((int)(-halfwidth - 1.0f), 0),
                QPoint((int)(halfwidth * mCells->tableWidth() + 1.0f),
                       (int)(mCells->cellHeight() * mCells->tableHeight() + 1.0f)));
}

void GridMeshCreator::writeVertices(GLfloat* aPosVec3, GLfloat* aTexVec2)
{
    const int w = mVertices->width();
    const int h = mVertices->height();
    GLuint index = 0;
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            auto& vtx = mVertices->vertex(x, y);
            if (vtx.isExist)
            {
                auto pos = vtx.posReduced();
                auto posPtr = &(aPosVec3[index * 3]);
                posPtr[0] = pos.x();
                posPtr[1] = pos.y();
                posPtr[2] = 0.0f;

                auto texPtr = &(aTexVec2[index * 2]);
                texPtr[0] = pos.x();
                texPtr[1] = pos.y();
                ++index;
            }
        }
    }
}

void GridMeshCreator::writeIndices(GLuint* aIndices)
{
    GLuint index = 0;

    for (int y = 0; y < mCells->tableHeight(); ++y)
    {
        for (int x = 0; x < mCells->tableWidth(); ++x)
        {
            auto& cell = mCells->cell(x, y);
            if (cell.isExist)
            {
                for (int k = 0; k < 3; ++k)
                {
                    aIndices[index] = cell.vtx[k]->index;
                    ++index;
                }
            }
        }
    }
}

void GridMeshCreator::writeConnections(HexaConnection* aDest)
{
    const int w = mVertices->width();
    const int h = mVertices->height();

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            auto& vtx = mVertices->vertex(x, y);
            if (vtx.isExist)
            {
                auto index = vtx.index;
                auto& connection = aDest[index];
                connection.clear();
                for (int i = 0; i < 6; ++i)
                {
                    const Vertex* cvtx = findConnectVertex(x, y, i);
                    if (cvtx) connection.id[i] = cvtx->index;
                }
            }
        }
    }
}

void GridMeshCreator::execute(const uint8* aPtr, const QSize& aSize, int aCellPx)
{
    Image image(aPtr, aSize);

    mCells.reset(new CellTable(aCellPx));
    mIndexCount = 3 * mCells->initCells(image);

    mVertices.reset(new VertexTable(
                        (mCells->tableWidth() + 1) / 2 + 1,
                        mCells->tableHeight() + 1));
    initPositionsOfVertices(*mVertices);

    mCells->connectCellsToVertices(*mVertices);

    setIndicesOfExistingVertices(*mVertices);

    reduceBurrs(*mVertices, image);
}

void GridMeshCreator::initPositionsOfVertices(VertexTable& aTable)
{
    const float cellWidth = mCells->cellWidth();
    const float cellHeight = mCells->cellHeight();
    const float cellHalfWidth = cellWidth * 0.5f;

    for (int y = 0; y < aTable.height(); ++y)
    {
        const bool zalign = (y % 2 == 0);
        const float zoffs = zalign ? -cellHalfWidth : 0.0f;

        for (int x = 0; x < aTable.width(); ++x)
        {
            auto& vtx = aTable.vertex(x, y);
            vtx.x = cellWidth * x + zoffs;
            vtx.y = cellHeight * y;
        }
    }
}

const GridMeshCreator::Vertex* GridMeshCreator::findConnectVertex(int aX, int aY, int aConnectId) const
{
    const bool zalign = (aY % 2 == 0);
    const int zoffset = zalign ? 0 : 1;
    int cellA = -1;
    int cellB = -1;

    const Vertex* c = mVertices->findVertex(aX, aY);
    if (!c || !c->isExist) return nullptr;

    switch (aConnectId)
    {
    case 0: cellA = 5; cellB = 0; aX += 1; break;
    case 1: cellA = 0; cellB = 1; aY += 1; aX += zoffset; break;
    case 2: cellA = 1; cellB = 2; aY += 1; aX += zoffset - 1; break;
    case 3: cellA = 2; cellB = 3; aX -= 1; break;
    case 4: cellA = 3; cellB = 4; aY -= 1; aX += zoffset - 1; break;
    case 5: cellA = 4; cellB = 5; aY -= 1; aX += zoffset; break;
    default: XC_ASSERT(0); break;
    }
    if (c->cell[cellA] == -1 && c->cell[cellB] == -1) return nullptr;

    const Vertex* v = mVertices->findVertex(aX, aY);
    if (!v || !v->isExist) return nullptr;
    return v;
}

void GridMeshCreator::setIndicesOfExistingVertices(VertexTable& aTable)
{
    GLuint index = 0;
    for (int y = 0; y < aTable.height(); ++y)
    {
        for (int x = 0; x < aTable.width(); ++x)
        {
            auto& vtx = aTable.vertex(x, y);
            if (vtx.isExist)
            {
                vtx.index = index;
                ++index;
            }
        }
    }
    mVertexCount = (int)index;
}

void GridMeshCreator::reduceBurrs(VertexTable& aTable, const Image& aImage)
{
    const float reduce = mCells->cellWidth();
    const float halfReduce = reduce * 0.5f;
    const int cellTableWidth = mCells->tableWidth();
    const int cellTableHeight = mCells->tableHeight();

    aTable.setReducingVectors(reduce);

    // avoid triangle flipping
    for (int y = 0; y < cellTableHeight; ++y)
    {
        for (int x = 0; x < cellTableWidth; ++x)
        {
            auto& cell = mCells->cell(x, y);
            if (!cell.isExist) continue;

            auto v0 = cell.vtx[0];
            auto v1 = cell.vtx[1];
            auto v2 = cell.vtx[2];

            if (v0->maxReduce <= 0.0f &&
                v1->maxReduce <= 0.0f &&
                v2->maxReduce <= 0.0f)
            {
                cell.nonReducing = true;
                cell.reducingFixed = true;
                continue;
            }

            const util::Segment2D seg0(v0->pos(), v0->reduceVec);
            const util::Segment2D seg1(v1->pos(), v1->reduceVec);
            const util::Segment2D seg2(v2->pos(), v2->reduceVec);

            if (util::MathUtil::areSegmentsFacingEachOther(seg0, seg1))
            {
                if (v0->maxReduce > 0.0f) v0->maxReduce = halfReduce;
                if (v1->maxReduce > 0.0f) v1->maxReduce = halfReduce;
            }
            if (util::MathUtil::areSegmentsFacingEachOther(seg0, seg2))
            {
                if (v0->maxReduce > 0.0f) v0->maxReduce = halfReduce;
                if (v2->maxReduce > 0.0f) v2->maxReduce = halfReduce;
            }
            if (util::MathUtil::areSegmentsFacingEachOther(seg1, seg2))
            {
                if (v1->maxReduce > 0.0f) v1->maxReduce = halfReduce;
                if (v2->maxReduce > 0.0f) v2->maxReduce = halfReduce;
            }
        }
    }

    // shorten reducing vectors if they are riding on a opaque pixels.
    for (int y = 0; y < aTable.height(); ++y)
    {
        for (int x = 0; x < aTable.width(); ++x)
        {
            auto& vtx = aTable.vertex(x, y);
            if (vtx.isExist)
            {
                if (vtx.maxReduce > 0.0f)
                {
                    vtx.reduceRate = 0.95f;
                    const float maxReduce = vtx.maxReduce;

                    for (int div = 0; div < 9; ++div)
                    {
                        auto pos = vtx.posReduced();

                        if (!aImage.hasSomeAlphaIn3x3((int)pos.x(), (int)pos.y()))
                        {
                            break;
                        }
                        vtx.maxReduce = (1.0f - (div + 1) * 0.125f) * maxReduce;
                    }
                }
            }
        }
    }

#if 1
    auto iw = aImage.size().width();
    auto ih = aImage.size().height();
    QScopedArrayPointer<uint8> useBits(new uint8[iw * ih]);
    QScopedArrayPointer<sint32> cellIds(new sint32[iw * ih]);

    for (int y = 0; y < ih; ++y)
    {
        auto line = y * iw;
        for (int x = 0; x < iw; ++x)
        {
            useBits[line + x] = 0;
            cellIds[line + x] = -1;
        }
    }

    for (int y = 0; y < cellTableHeight; ++y)
    {
        const int line = y * cellTableWidth;
        for (int x = 0; x < cellTableWidth; ++x)
        {
            const int cellIdx = line + x;
            auto& cell = mCells->cell(x, y);

            if (!cell.isExist ||
                cell.nonReducing ||
                cell.reducingFixed) continue;

            util::TriangleRasterizer rasterizer(
                        cell.vtx[0]->pos(),
                        cell.vtx[1]->pos(),
                        cell.vtx[2]->pos());

            while (rasterizer.hasNext())
            {
                auto scan = rasterizer.nextLine();
                for (int rx = scan.xbgn; rx < scan.xend; ++rx)
                {
                    auto ry = scan.y;

                    if (aImage.hasAlpha(rx, ry))
                    {
                        auto pix = rx + ry * iw;
                        useBits[pix] = 1;
                        cellIds[pix] = cellIdx;
                    }
                }
            }
        }
    }

    const int divEnd = 9;
    for (int div = 0; div < divEnd; ++div)
    {
        for (int y = 0; y < cellTableHeight; ++y)
        {
            for (int x = 0; x < cellTableWidth; ++x)
            {
                auto& cell = mCells->cell(x, y);

                if (!cell.isExist ||
                    cell.nonReducing ||
                    cell.reducingFixed) continue;

                util::TriangleRasterizer rasterizer(
                            cell.vtx[0]->posReduced(),
                            cell.vtx[1]->posReduced(),
                            cell.vtx[2]->posReduced());

                while (rasterizer.hasNext())
                {
                    auto scan = rasterizer.nextLine();
                    for (int rx = scan.xbgn; rx < scan.xend; ++rx)
                    {
                        auto ry = scan.y;
                        if (rx < 0 || iw <= rx || ry < 0 || ih <= ry)
                        {
                            continue;
                        }

                        for (int iy = -1; iy < 2; ++iy)
                        {
                            auto ay = ry + iy;
                            if (ay < 0 || ih <= ay) continue;

                            for (int ix = -1; ix < 2; ++ix)
                            {
                                auto ax = rx + ix;
                                if (ax < 0 || iw <= ax) continue;

                                auto pix = ax + ay * iw;
                                uint8& usebit = useBits[pix];
                                if (usebit == 1)
                                {
                                    usebit = 2;
                                }
                            }
                        }
                        /*
                        auto pix = rx + ry * iw;
                        uint8& usebit = useBits[pix];
                        if (usebit == 1)
                        {
                            usebit = 2;
                        }
                        */
                    }
                }

                cell.reducingFixed = true;
            }
        }

        if (div == divEnd - 1) break;

        const float nextRate = 1.0f - (div + 1) * 0.125f;

        for (int y = 0; y < ih; ++y)
        {
            auto line = y * iw;
            for (int x = 0; x < iw; ++x)
            {
                auto pix = line + x;
                uint8& usebit = useBits[pix];
                if (usebit == 1)
                {
                    auto cellIdx = cellIds[pix];
                    const int cellY = cellIdx / cellTableWidth;
                    const int cellX = cellIdx - cellY * cellTableWidth;
                    Cell* cell = &(mCells->cell(cellX, cellY));

#if 1
                    if (cell && !cell->nonReducing)
                    {
                        cell->reducingFixed = false;
                        cell->vtx[0]->reduceRate = nextRate;
                        cell->vtx[1]->reduceRate = nextRate;
                        cell->vtx[2]->reduceRate = nextRate;
                    }
#else
                    Cell* arounds[4] = {
                        cell,
                        mCells->findExistingCell(cellX - 1, cellY),
                        mCells->findExistingCell(cellX + 1, cellY),
                        mCells->findExistingCell(cellX, cellY + (cell->inverted ? -1 : 1))
                    };

                    for (int i = 0; i < 4; ++i)
                    {
                        Cell* a = arounds[i];
                        if (a && !a->nonReducing)
                        {
                            a->reducingFixed = false;
                            a->vtx[0]->reduceRate = nextRate;
                            a->vtx[1]->reduceRate = nextRate;
                            a->vtx[2]->reduceRate = nextRate;
                        }
                    }
#endif
                }
            }
        }
    }

    mVertices->shortenReducingVectorsOnePixel();

#endif
}

} // namespace img
