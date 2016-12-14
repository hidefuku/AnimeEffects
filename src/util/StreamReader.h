#ifndef UTIL_STREAMREADER_H
#define UTIL_STREAMREADER_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cassert>
#include <vector>
#include "XCType.h"

namespace util
{

struct BigSwapper
{
    template<typename T>
    static inline T swap(T aSrc) { return qFromBigEndian(aSrc); }
};

struct LittleSwapper
{
    template<typename T>
    static inline T swap(T aSrc) { return qFromLittleEndian(aSrc); }
};

template<typename tSwapper>
class StreamReader
{
    std::istream& mIo;
    std::ios::pos_type mStart;
    std::vector<uint8> mBuf;

public:
    StreamReader(std::istream& aIo)
        : mIo(aIo)
        , mStart(aIo.tellg())
    {
    }

    bool isFailed() const
    {
        return mIo.fail();
    }

    void skipWhile(char aSkipLetter)
    {
        while(mIo)
        {
            char letter = mIo.peek();
            if (letter != aSkipLetter)
            {
                break;
            }
            else
            {
                mIo.get();
            }
        }
    }

    void skipTo(std::ios::pos_type aEnd)
    {
        skip(aEnd - tellg());
    }

    void skip(int aBytes)
    {
        mIo.seekg(aBytes, std::ios::cur);
    }

    bool skipZeroArea(int aBytes)
    {
        while (aBytes)
        {
            if (!mIo || readByte() != 0)
            {
                return false;
            }
            --aBytes;
        }
        return true;
    }

    void skip4Bound()
    {
        int remain = tellg() % 4;
        if (remain)
        {
            skip(4 - remain);
        }
    }

    std::string readString()
    {
        std::stringstream ss;
        while (mIo)
        {
            char letter = mIo.get();
            if (letter == 0) break;
            ss << letter;
        }
        return ss.str();
    }

    std::string readStringWithLimit(int aLimit)
    {
        if (aLimit <= 0) return std::string();

        std::stringstream ss;
        while (mIo)
        {
            char letter = mIo.get();
            if (letter == 0) break;
            ss << letter;
            if (--aLimit <= 0) break;
        }
        return ss.str();
    }

    std::string readString(int aBytes)
    {
        if (aBytes <= 0) return std::string();
        mBuf.resize(aBytes);
        mIo.read((char*)&mBuf[0], aBytes);
        return std::string(mBuf.begin(), mBuf.end());
    }

    const std::vector<uint8>& readVector(int aBytes)
    {
        mBuf.resize(aBytes);
        mIo.read((char*)&mBuf[0], aBytes);
        return mBuf;
    }

    void readBuf(uint8* aBuf, size_t aSize)
    {
        mIo.read((char*)aBuf, aSize);
    }

    uint16 readUInt16()
    {
        uint16 x;
        mIo.read((char*)(&x), 2);
        return tSwapper::swap(x);
    }

    sint16 readSInt16()
    {
        sint16 x;
        mIo.read((char*)(&x), 2);
        return tSwapper::swap(x);
    }

    uint32 readUInt32()
    {
        uint32 x;
        mIo.read((char*)(&x), 4);
        return tSwapper::swap(x);
    }

    sint32 readSInt32()
    {
        sint32 x;
        mIo.read((char*)(&x), 4);
        return tSwapper::swap(x);
    }

    float32 readFloat32()
    {
        uint32 x;
        mIo.read((char*)(&x), 4);
        x = tSwapper::swap(x);
        void* xp = &x; // for avoid warning
        return *((float32*)xp);
    }

    uint64 readUInt64()
    {
        uint64 x;
        mIo.read((char*)(&x), 8);
        return tSwapper::swap(x);
    }

    sint64 readSInt64()
    {
        sint64 x;
        mIo.read((char*)(&x), 8);
        return tSwapper::swap(x);
    }

    float64 readFloat64()
    {
        uint64 x;
        mIo.read((char*)(&x), 8);
        x = tSwapper::swap(x);
        return *((float64*)&x);
    }

    uint8 readByte()
    {
        return mIo.get();
    }

    std::ios::pos_type tellg()
    {
        return mIo.tellg() - mStart;
    }
};

typedef StreamReader<BigSwapper> BEStreamReader;
typedef StreamReader<LittleSwapper> LEStreamReader;

} // namespace util

#endif // UTIL_STREAMREADER_H

