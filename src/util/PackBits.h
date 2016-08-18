#ifndef UTIL_PACKBITS_H
#define UTIL_PACKBITS_H

#include "XC.h"

namespace util
{

class PackBits
{
public:
    PackBits();

    static size_t worstEncodedSize(size_t aSrcSize);

    size_t encode(const XCMemBlock& aSrc, uint8* aDst);

    bool decode(const XCMemBlock& aSrc, XCMemBlock& aDst);
};

} // namespace util

#endif // UTIL_PACKBITS_H
