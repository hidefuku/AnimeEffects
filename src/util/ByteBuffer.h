#ifndef UTIL_BUFFER
#define UTIL_BUFFER

#include <cstring>
#include "XC.h"

namespace util
{

class ByteBuffer
{
public:
    ByteBuffer();
    ByteBuffer(const ByteBuffer& aRhs);
    ByteBuffer& operator=(const ByteBuffer& aRhs);
    virtual ~ByteBuffer();

    void grab(const XCMemBlock& aBlock);
    void grab(uint8* aPtr, size_t aSize);
    void alloc(size_t aSize);
    void free();

    explicit operator bool() const { return mBlock.data; }
    const XCMemBlock& block() const { return mBlock; }
    uint8* data() { return mBlock.data; }
    const uint8* data() const { return mBlock.data; }
    size_t size() const { return mBlock.size; }

private:
    XCMemBlock mBlock;
};

} // namespace util

#endif // UTIL_BUFFER

