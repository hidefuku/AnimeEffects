#ifndef CORE_FRAME
#define CORE_FRAME

namespace core
{

class Frame
{
public:
    Frame()
        : mValue(0)
        , mMilli(0)
    {
    }

    explicit Frame(int aValue)
        : mValue(aValue)
        , mMilli(0)
    {
    }

    static Frame fromDecimal(float aValue)
    {
        Frame frame;
        frame.setDecimal(aValue);
        return frame;
    }

    inline bool operator==(const Frame& aRhs) const
    {
        return mValue == aRhs.mValue && mMilli == aRhs.mMilli;
    }

    inline bool operator<(const Frame& aRhs) const
    {
        if (mValue == aRhs.mValue) return mMilli < aRhs.mMilli;
        return mValue < aRhs.mValue;
    }

    inline bool operator>(const Frame& aRhs) const
    {
        if (mValue == aRhs.mValue) return mMilli > aRhs.mMilli;
        return mValue > aRhs.mValue;
    }

    inline bool operator<=(const Frame& aRhs) const { return !(*this > aRhs); }
    inline bool operator>=(const Frame& aRhs) const { return !(*this < aRhs); }
    inline bool operator<(int aRhs) const { return *this < Frame(aRhs); }
    inline bool operator>(int aRhs) const { return *this > Frame(aRhs); }
    inline bool operator<=(int aRhs) const { return *this <= Frame(aRhs); }
    inline bool operator>=(int aRhs) const { return *this >= Frame(aRhs); }

    inline void set(int aValue)
    {
        mValue = aValue;
        mMilli = 0;
    }

    inline void setDecimal(float aValue)
    {
        mValue = (int)aValue;
        mMilli = (int)(1000.0f * (aValue - mValue));
    }

    inline void add(int aValue)
    {
        mValue += aValue;
    }

    inline void addDecimal(float aValue)
    {
        setDecimal(getDecimal() + aValue);
    }

    inline int get() const
    {
        return mValue;
    }

    inline float getDecimal() const
    {
        return mValue + (mMilli / 1000.0f);
    }

    inline bool hasFraction() const
    {
        return mMilli != 0;
    }

    void clamp(int aMin, int aMax)
    {
        const float v = getDecimal();
        if (v < aMin)
        {
            set(aMin);
        }
        else if (aMax < v)
        {
            set(aMax);
        }
    }

    inline Frame added(int aValue) const
    {
        Frame frame = *this;
        frame.mValue += aValue;
        return frame;
    }

    inline Frame integrated() const
    {
        return Frame(mValue);
    }

    struct SerialValue { int value; int milli; };

    SerialValue serialValue() const
    {
        const SerialValue v = { mValue, mMilli };
        return v;
    }

    void setSerialValue(const SerialValue& aSerial)
    {
        mValue = aSerial.value;
        mMilli = aSerial.milli;
    }

private:
    int mValue;
    int mMilli;
};

} // namespace core

#endif // CORE_FRAME

