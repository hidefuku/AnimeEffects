#ifndef UTIL_PLACEPOINTER
#define UTIL_PLACEPOINTER

#include "XC.h"

namespace util
{

template<typename tObject>
class PlacePointer
{
    union Align
    {
        tObject* ptr;
        long long dummy0;
        size_t dummy1;
        int dummy2;
    };

    Align mAlign;
    /*alignas(tObject)*/ char mBuffer[sizeof(tObject)]; // @todo msvc c++11 bug?

public:
    PlacePointer()
        : mAlign()
    {
        mAlign.ptr = NULL;
    }

    PlacePointer(const PlacePointer<tObject>& aRhs)
        : mAlign()
    {
        mAlign.ptr = NULL;
        if (aRhs.mAlign.ptr)
        {
            construct(*aRhs.mAlign.ptr);
        }
    }

    ~PlacePointer()
    {
        destruct();
    }

    PlacePointer<tObject>& operator=(const PlacePointer<tObject>& aRhs)
    {
        if (!mAlign.ptr)
        {
            construct(*aRhs.mAlign.ptr);
        }
        else
        {
            *mAlign.ptr = *aRhs.mAlign.ptr;
        }
        return *this;
    }

    template<typename... tArgs>
    void construct(tArgs&&... aArgs)
    {
        destruct();
        mAlign.ptr = new (mBuffer) tObject(std::forward<tArgs>(aArgs)...);
    }

    void destruct()
    {
        if (mAlign.ptr)
        {
            mAlign.ptr->~tObject();
            mAlign.ptr = NULL;
        }
    }

    tObject* operator->() const
    {
        XC_PTR_ASSERT(mAlign.ptr);
        return mAlign.ptr;
    }

    tObject& operator*() const
    {
        XC_PTR_ASSERT(mAlign.ptr);
        return *mAlign.ptr;
    }

    tObject* get() const
    {
        return mAlign.ptr;
    }

    explicit operator bool() const
    {
        return mAlign.ptr != NULL;
    }

    bool operator==(const tObject* aRhs)
    {
        return mAlign.ptr == aRhs;
    }

    bool operator==(const PlacePointer<tObject>& aRhs)
    {
        return mAlign.ptr == aRhs.mAlign.ptr;
    }
};

} // namespace util

#endif // UTIL_PLACEPOINTER

