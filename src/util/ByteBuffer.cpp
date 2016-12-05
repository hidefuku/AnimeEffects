#include "util/ByteBuffer.h"

namespace util
{

ByteBuffer::ByteBuffer()
    : mBlock()
{
}

ByteBuffer::ByteBuffer(const ByteBuffer& aRhs)
    : mBlock()
{
    if (aRhs.size())
    {
        alloc(aRhs.size());
        memcpy(mBlock.data, aRhs.data(), aRhs.size());
    }
}

ByteBuffer& ByteBuffer::operator=(const ByteBuffer& aRhs)
{
    free();
    if (aRhs.size())
    {
        alloc(aRhs.size());
        memcpy(mBlock.data, aRhs.data(), aRhs.size());
    }
    return *this;
}

ByteBuffer::~ByteBuffer()
{
    free();
}

void ByteBuffer::grab(const XCMemBlock& aBlock)
{
    free();
    mBlock = aBlock;
}

void ByteBuffer::grab(uint8* aPtr, size_t aSize)
{
    free();
    mBlock = XCMemBlock(aPtr, aSize);
}

void ByteBuffer::alloc(size_t aSize)
{
    XC_ASSERT(aSize > 0);
    if (mBlock.size != aSize)
    {
        free();
        mBlock.size = aSize;
        mBlock.data = new uint8[mBlock.size];
    }
}

void ByteBuffer::free()
{
    if (mBlock.data)
    {
        delete[] mBlock.data;
        mBlock = XCMemBlock();
    }
}

} // namespace util
