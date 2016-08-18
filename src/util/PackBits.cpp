#include "util/PackBits.h"

namespace util
{

PackBits::PackBits()
{
}

size_t PackBits::worstEncodedSize(size_t aSrcSize)
{
    return aSrcSize + (aSrcSize / 128) + 1;
}

size_t PackBits::encode(const XCMemBlock& aSrc, uint8* aDst)
{
    const uint8* end = aSrc.data + aSrc.size;
    const uint8* p = aSrc.data;
    uint8* dp = aDst;
    long long int remains = aSrc.size;

    while (remains > 0)
    {
        const uint8* mark = p;
        const uint8* markMax = mark + (remains < 128 ? remains : 128);

        // found three duplicated value
        if (mark <= (end - 3) && mark[0] == mark[1] && mark[1] == mark[2])
        {
            // scan
            p += 3;
            while (p < markMax && *p == mark[0]) ++p;

            const int count = (int)(p - mark);
            *dp++ = 1 + 256 - count;
            *dp++ = mark[0];
            remains -= count;
        }
        else
        {
            while (p < markMax)
            {
                if(p <= (end - 3) && p[0] == p[1] && p[1] == p[2]) break;
                ++p;
            }
            const int count = (int)(p - mark);
            *dp++ = count - 1;
            memcpy(dp, mark, count);
            dp += count;
            remains -= count;
        }
    }
    return dp - aDst;
}

bool PackBits::decode(const XCMemBlock& aSrc, XCMemBlock& aDst)
{
    const uint8* end = aSrc.data + aSrc.size;
    const uint8* p = aSrc.data;
    uint8* dp = aDst.data;
    size_t dlen = aDst.size;
    size_t x = 0;

    while (p < end)
    {
        // packet header
        const char head = (char)(*p);

        if (head < 0)
        {
            // noop operation
            if (head == -128) continue;

            // continuos operation
            const size_t count = -head + 1;
            x += count;

            if (x > dlen)
            {
                //PACKBITS_DUMP("decode packbits error 1: %lu", x);
                return false;
            }

            const uint8 value = *(++p);

            for (size_t i = 0; i < count; ++i, ++dp)
            {
                *dp = value;
            }
        }
        else
        {
            // no continuos operation
            const size_t count = head + 1;
            x += count;
            if (x > dlen)
            {
                //PACKBITS_DUMP("decode packbits error 2: %lu", x);
                return false;
            }
            if ((p + count) >= end)
            {
                //PACKBITS_DUMP("decode packbits error 3");
                return false;
            }

            for (size_t i = 0; i < count; ++i, ++dp)
            {
                *dp = *(++p);
            }
        }

        ++p;
    }

    if (x != dlen)
    {
        //PACKBITS_DUMP("decode packbits error 4: %lu", x);
        return false;
    }

    return true;
}

} // namespace util
