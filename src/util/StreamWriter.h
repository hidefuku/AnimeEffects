#ifndef UTIL_STREAMWRITER
#define UTIL_STREAMWRITER

#include <ostream>
#include <array>
#include "XC.h"

namespace util
{

class StreamWriter
{
    std::ostream& mOut;

public:
    StreamWriter(std::ostream& aOut)
        : mOut(aOut)
    {
    }

    template<typename tValue> void write(tValue aValue)
    {
        tValue value = XC_TO_LITTLE_ENDIAN(aValue);
        mOut.write((char*)(&value), sizeof(tValue));
    }

    template<typename tValue, int tCount> void write(std::array<tValue, tCount> aValues)
    {
        for (auto v : aValues)
        {
            tValue value = XC_TO_LITTLE_ENDIAN(v);
            mOut.write((char*)(&value), sizeof(tValue));
        }
    }

    template<typename tValue> void writeTo(std::ios::pos_type aPos, tValue aValue)
    {
        std::ios::pos_type current = mOut.tellp();
        mOut.seekp(aPos);
        write(aValue);
        mOut.seekp(current);
    }

    void writeZero(int aLength)
    {
        for (int i = 0; i < aLength; ++i) mOut.put((char)0);
    }

    void writeString(const std::string& aString, int aAlignment)
    {
        auto size = aString.size();
        if (size > 0)
        {
            mOut.write(aString.c_str(), size);
        }
        writeZero(1);
        auto pad = (size + 1) % aAlignment;
        if (pad) writeZero(aAlignment - (int)pad);
    }

    void writeBytes(const XCMemBlock& aBlock, int aAlignment)
    {
        mOut.write((const char*)aBlock.data, aBlock.size);
        auto pad = aBlock.size % aAlignment;
        if (pad) writeZero(aAlignment - (int)pad);
    }

    std::ostream::pos_type currentPos() const
    {
        return mOut.tellp();
    }

    std::ostream::pos_type reserveLength()
    {
        auto pos = mOut.tellp();
        write((uint64)0);
        return pos;
    }

    void alignFrom(std::ios::pos_type aPos, int aAlignment)
    {
        auto pad = (currentPos() - aPos) % aAlignment;
        if (pad) writeZero(aAlignment - (int)pad);
    }

    void writeLength(std::ios::pos_type aPos)
    {
        size_t length = static_cast<size_t>(mOut.tellp() - aPos) - 8;
        writeTo(aPos, (uint64)length);
    }

    bool isFailed() const
    {
        return mOut.fail();
    }
};

} // namespace util

#endif // UTIL_STREAMWRITER

