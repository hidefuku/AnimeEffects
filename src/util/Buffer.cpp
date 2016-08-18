#include "util/Buffer.h"

namespace util
{

Buffer::Buffer()
    : mBlock()
{
}

Buffer::Buffer(const Buffer& aRhs)
    : mBlock()
{
    if (aRhs.size())
    {
        alloc(aRhs.size());
        memcpy(mBlock.data, aRhs.data(), aRhs.size());
    }
}

Buffer& Buffer::operator=(const Buffer& aRhs)
{
    free();
    if (aRhs.size())
    {
        alloc(aRhs.size());
        memcpy(mBlock.data, aRhs.data(), aRhs.size());
    }
    return *this;
}

Buffer::~Buffer()
{
    free();
}

void Buffer::grab(const XCMemBlock& aBlock)
{
    free();
    mBlock = aBlock;
}

void Buffer::alloc(size_t aSize)
{
    XC_ASSERT(aSize > 0);
    if (mBlock.size != aSize)
    {
        free();
        mBlock.size = aSize;
        mBlock.data = new uint8[mBlock.size];
    }
}

void Buffer::free()
{
    if (mBlock.data)
    {
        delete[] mBlock.data;
        mBlock = XCMemBlock();
    }
}

} // namespace util
