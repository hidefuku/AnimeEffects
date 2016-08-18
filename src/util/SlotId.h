#ifndef UTIL_SLOTID
#define UTIL_SLOTID

namespace util
{

class SlotId
{
public:
    SlotId() : mValue() {}
    SlotId(const void* aValue) : mValue(aValue) {}
    SlotId& operator=(const void* aValue) { mValue = aValue; return *this; }
    const void* value() const { return mValue; }
    bool isValid() const { return mValue != NULL; }
private:
    const void* mValue;
};

}

#endif // UTIL_SLOTID

