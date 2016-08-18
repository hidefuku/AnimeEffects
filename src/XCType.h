#ifndef XCTYPE_H
#define XCTYPE_H

typedef char sint8;
typedef short sint16;
typedef int sint32;
typedef long long sint64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef float float32;
typedef double float64;

#include <QtEndian>
#define XC_FROM_BIG_ENDIAN(x) qFromBigEndian(x)
#define XC_FROM_LITTLE_ENDIAN(x) qFromLittleEndian(x)
#define XC_TO_BIG_ENDIAN(x) qToBigEndian(x)
#define XC_TO_LITTLE_ENDIAN(x) qToLittleEndian(x)

struct XCMemBlock
{
    XCMemBlock() : data(), size() {}
    XCMemBlock(uint8* aData, size_t aSize) : data(aData), size(aSize) {}
    uint8* data;
    size_t size;
};

template<typename tValue>
tValue xc_clamp(tValue aValue, tValue aMin, tValue aMax)
{
    return (aValue > aMax) ? aMax : ((aValue < aMin) ? aMin : aValue);
}

template<typename tValue>
tValue xc_divide(tValue aNum, tValue aDenom, tValue aDenomMin, tValue aDefault)
{
    const tValue absDenom = (aDenom >= 0.0f) ? aDenom : -aDenom;
    return (absDenom >= aDenomMin) ? (aNum / aDenom) : aDefault;
}

template<typename tValue>
bool xc_contains(tValue aValue, tValue aMin, tValue aMax)
{
    return aMin <= aValue && aValue <= aMax;
}

template<typename tValue>
tValue xc_decrease(tValue aValue, tValue aDec)
{
    if (aValue >= 0.0f)
    {
        tValue v = aValue - aDec;
        return v > 0.0f ? v : 0.0f;
    }
    else
    {
        tValue v = aValue + aDec;
        return v < 0.0f ? v : 0.0f;
    }
}

#endif // XCTYPE_H
