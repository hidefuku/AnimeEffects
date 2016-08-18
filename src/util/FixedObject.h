#ifndef UTIL_FIXEDOBJECT_H
#define UTIL_FIXEDOBJECT_H

namespace util
{

template<typename tObject>
class FixedObject
{
    typedef void (FixedObject::*SafeBoolType)() const;
    void dummyFuncForSafeBoolIdiom() const {}

    void makeSureBuffer() { if (!mObj) { mObj = (tObject*)std::malloc(sizeof(tObject)); } }
    void freeBuffer() { if (mObj) { std::free(mObj); mObj = nullptr; } }
    void preConstruct() { makeSureBuffer(); destruct(); mIsConstructed = true; }

    tObject* mObj;
    bool mIsConstructed;

public:
    FixedObject()
        : mObj(nullptr)
        , mIsConstructed(false)
    {
    }

    ~FixedObject()
    {
        destruct();
        freeBuffer();
    }

    // msvc 2012 does not support variable arguments
#if 0
    template<typename... tArgs>
    tObject* construct(tArgs&&... aArgs)
    {
        preConstruct();
        return new (mObj) tObj(std::forward<tArgs>(aArgs)...);
    }
#else
    tObject* construct()
        { preConstruct(); return new (mObj) tObject(); }

    template<typename tArg0>
    tObject* construct(tArg0&& aArg0)
        { preConstruct(); return new (mObj) tObject(aArg0); }

    template<typename tArg0, typename tArg1>
    tObject* construct(tArg0&& aArg0, tArg1&& aArg1)
        { preConstruct(); return new (mObj) tObject(aArg0, aArg1); }

    template<typename tArg0, typename tArg1, typename tArg2>
    tObject* construct(tArg0&& aArg0, tArg1&& aArg1, tArg2&& aArg2)
        { preConstruct(); return new (mObj) tObject(aArg0, aArg1, aArg2); }

    template<typename tArg0, typename tArg1, typename tArg2, typename tArg3>
    tObject* construct(tArg0&& aArg0, tArg1&& aArg1, tArg2&& aArg2, tArg3&& aArg3)
        { preConstruct(); return new (mObj) tObject(aArg0, aArg1, aArg2, aArg3); }
#endif

    void destruct()
    {
        if (mIsConstructed)
        {
            mObj->~tObject();
            mIsConstructed = false;
        }
    }

    operator SafeBoolType() const
    {
        return mObj ? &FixedObject::dummyFuncForSafeBoolIdiom : 0;
    }

    tObject* operator->() const
    {
        return mObj;
    }

    tObject& operator*() const
    {
        return *mObj;
    }

    tObject* get() const
    {
        return mObj;
    }
};

} // namespace util

#endif // UTIL_FIXEDOBJECT_H
