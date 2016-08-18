#ifndef UTIL_SIGNALER_H
#define UTIL_SIGNALER_H

#include <list>
#include <functional>
#include "util/SlotId.h"

namespace util
{

template<typename tDummy>
class Signaler;

template<typename tResult, typename... tArgs>
class Signaler<tResult(tArgs...)>
{
public:
    typedef std::function<tResult(tArgs...)> FunctionType;
    typedef std::list<FunctionType*> ListType;
    typedef FunctionType* IdType;

    Signaler()
        : mFunctions()
    {
    }

    ~Signaler()
    {
        for (auto function : mFunctions)
        {
            delete function;
        }
    }

    // for std::function, lambda
    SlotId connect(const FunctionType& aFunction)
    {
        IdType id = new FunctionType(aFunction);
        mFunctions.push_back(id);
        return SlotId(id);
    }

    // for member function
    template<typename tObj>
    SlotId connect(tObj* aObj, tResult(tObj::*aFunc)(tArgs...))
    {
        IdType id = new FunctionType([aObj, aFunc](tArgs...args)->tResult{ (aObj->*aFunc)(args...); });
        mFunctions.push_back(id);
        return SlotId(id);
    }

    void disconnect(SlotId aId)
    {
        typename ListType::iterator itr = std::find(mFunctions.begin(), mFunctions.end(), static_cast<const FunctionType*>(aId.value()));
        if (itr != mFunctions.end())
        {
            delete *itr;
            mFunctions.erase(itr);
        }
    }

    void operator()(tArgs... aArgs)
    {
        for (auto function : mFunctions)
        {
            (*function)(aArgs...);
        }
    }

private:
    ListType mFunctions;
};

} // namespace util

#endif // UTIL_SIGNALER_H

