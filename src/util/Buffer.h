#ifndef UTIL_BUFFER
#define UTIL_BUFFER

#include <cstring>
#include "XC.h"

namespace util
{

class Buffer
{
public:
    Buffer();
    Buffer(const Buffer& aRhs);
    Buffer& operator=(const Buffer& aRhs);
    ~Buffer();

    void grab(const XCMemBlock& aBlock);
    void alloc(size_t aSize);
    void free();

    const XCMemBlock& block() const { return mBlock; }

    uint8* data() { return mBlock.data; }
    const uint8* data() const { return mBlock.data; }

    size_t size() const { return mBlock.size; }

private:
    XCMemBlock mBlock;
};

} // namespace util

#endif // UTIL_BUFFER

