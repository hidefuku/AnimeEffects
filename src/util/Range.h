#ifndef UTIL_RANGE
#define UTIL_RANGE

namespace util
{

class Range
{
public:
    Range() : mMin(), mMax() {}
    Range(int aMin, int aMax) : mMin(aMin), mMax(aMax) {}

    int min() const { return mMin; }
    int max() const { return mMax; }
    int diff() const { return mMax - mMin; }
    bool isNegative() const { return mMin > mMax; }
    bool contains(int aValue) const { return mMin <= aValue && aValue <= mMax; }
    bool contains(float aValue) const { return mMin <= aValue && aValue <= mMax; }

    void setMin(int aMin) { mMin = aMin; }
    void setMax(int aMax) { mMax = aMax; }

private:
    int mMin;
    int mMax;
};

} // namespace util

#endif // UTIL_RANGE

