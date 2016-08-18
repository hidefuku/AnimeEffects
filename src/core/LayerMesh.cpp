#include "XC.h"
#include "gl/Global.h"
#include "core/LayerMesh.h"

namespace core
{

LayerMesh::MeshBuffer::GLBinder::GLBinder()
{
    gl::Global::makeCurrent();
}

LayerMesh::MeshBuffer::MeshBuffer()
    : workPositions(GL_TRANSFORM_FEEDBACK_BUFFER)
    , workXArrows(GL_TRANSFORM_FEEDBACK_BUFFER)
    , workYArrows(GL_TRANSFORM_FEEDBACK_BUFFER)
    , outPositions(GL_ARRAY_BUFFER)
    , outXArrows(GL_ARRAY_BUFFER)
    , outYArrows(GL_ARRAY_BUFFER)
    , vtxCount(0)
{
}

void LayerMesh::MeshBuffer::reserve(int aVtxCount)
{
    XC_ASSERT(aVtxCount >= 0);
    if (vtxCount != aVtxCount)
    {
        vtxCount = aVtxCount;

        const int reserve = vtxCount > 0 ? vtxCount : 1; // fail safe code
        workPositions.resetData<gl::Vector3>(reserve, GL_STREAM_COPY);
        workXArrows.resetData<gl::Vector3>(reserve, GL_STREAM_COPY);
        workYArrows.resetData<gl::Vector3>(reserve, GL_STREAM_COPY);
        outPositions.resetData<gl::Vector3>(reserve, GL_STREAM_COPY);
        outXArrows.resetData<gl::Vector3>(reserve, GL_STREAM_COPY);
        outYArrows.resetData<gl::Vector3>(reserve, GL_STREAM_COPY);
    }
}

LayerMesh::MeshBuffer::~MeshBuffer()
{
    gl::Global::makeCurrent();
}

LayerMesh::ArrayedConnection::ArrayedConnection()
    : positions()
    , positionCount()
    , vertexRange()
{
}

void LayerMesh::ArrayedConnection::resetPositions()
{
    positions.reset(new gl::Vector2[ArrayedConnection::kMaxCount]);
    positionCount = 0;
    vertexRange = util::Range();
}

void LayerMesh::ArrayedConnection::pushPosition(const gl::Vector2& aPos)
{
    XC_ASSERT(positionCount < kMaxCount);
    positions[positionCount] = aPos;
    ++positionCount;
}

LayerMesh::ArrayedConnectionList::ArrayedConnectionList()
    : indexRanges()
    , blocks()
    , indexRangeCount()
    , useBlockCount(0)
{
}

LayerMesh::ArrayedConnectionList::~ArrayedConnectionList()
{
    destroyBlocks();
}

void LayerMesh::ArrayedConnectionList::clearBlocks()
{
    useBlockCount = 0;
}

void LayerMesh::ArrayedConnectionList::destroyBlocks()
{
    qDeleteAll(blocks);
    blocks.clear();
    useBlockCount = 0;
}

void LayerMesh::ArrayedConnectionList::destroyUnuseBlocks()
{
    int index = 0;
    for (auto itr = blocks.begin(); itr != blocks.end();)
    {
        if (index < useBlockCount)
        {
            ++itr;
        }
        else
        {
            auto block = *itr;
            itr = blocks.erase(itr);
            delete block;
        }
        ++index;
    }
}

LayerMesh::ArrayedConnection* LayerMesh::ArrayedConnectionList::pushNewBlock()
{
    ++useBlockCount;
    const int blockCount = blocks.count();

    if (blockCount < useBlockCount)
    {
        blocks.push_back(new ArrayedConnection());
        blocks.back()->resetPositions();
        return blocks.back();
    }
    else
    {
        auto reuseBlock = blocks.at(useBlockCount - 1);
        reuseBlock->resetPositions();
        return reuseBlock;
    }
}

void LayerMesh::ArrayedConnectionList::resetIndexRanges(int aCount)
{
    if (indexRangeCount != aCount)
    {
        indexRangeCount = aCount;
        if (aCount > 0)
        {
            indexRanges.reset(new gl::Vector2I[aCount]);
        }
        else
        {
            indexRanges.reset();
        }
    }
}

LayerMesh::ArrayedConnectionWriter::ArrayedConnectionWriter(ArrayedConnectionList& aList, int aVertexCount)
    : mList(aList)
    , mVertexCount(aVertexCount)
    , mCurBlock()
    , mIndex(-1)
{
    mList.resetIndexRanges(aVertexCount);
    mList.clearBlocks();

    mCurBlock = mList.pushNewBlock();
    mCurBlock->vertexRange.setMin(0);
}

LayerMesh::ArrayedConnectionWriter::~ArrayedConnectionWriter()
{
    if (mVertexCount > 0 && mCurBlock)
    {
        mCurBlock->vertexRange.setMax(mVertexCount - 1);
    }
    mList.destroyUnuseBlocks();
}

void LayerMesh::ArrayedConnectionWriter::beginOneVertex(int aMaxConnectionCount)
{
    XC_ASSERT(aMaxConnectionCount < ArrayedConnection::kMaxCount);

    ++mIndex;

    // setup positions
    if (mCurBlock->positionCount >= ArrayedConnection::kMaxCount - aMaxConnectionCount)
    {
        mCurBlock->vertexRange.setMax(mIndex - 1);

        mCurBlock = mList.pushNewBlock();
        mCurBlock->vertexRange.setMin(mIndex);
    }

    mList.indexRanges[mIndex].x = mCurBlock->positionCount;
    mList.indexRanges[mIndex].y = 0;
}

void LayerMesh::ArrayedConnectionWriter::pushPosition(const gl::Vector2& aPos)
{
    mCurBlock->pushPosition(aPos);
    ++(mList.indexRanges[mIndex].y);
}

} // namspace core
