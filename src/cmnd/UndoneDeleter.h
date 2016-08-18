#ifndef CMND_UNDONEDELETER
#define CMND_UNDONEDELETER

#include "util/NonCopyable.h"

namespace cmnd
{

template<typename tObj>
class UndoneDeleter : private util::NonCopyable
{
    tObj* mObj;
    bool mDone;

public:
    UndoneDeleter()
        : mObj()
        , mDone()
    {}

    explicit UndoneDeleter(tObj* aObj)
        : mObj(aObj)
        , mDone()
    {}

    ~UndoneDeleter()
    {
        if (mObj && !mDone) { delete mObj; }
    }

    void set(tObj* aObj) { mObj = aObj; }
    tObj* get() const { return mObj; }
    tObj* operator->() const { return mObj; }
    explicit operator bool() const { return mObj != nullptr; }
    bool operator==(const tObj* aRhs) { return mObj == aRhs; }
    bool operator==(const UndoneDeleter<tObj>& aRhs) { return mObj == aRhs.mObj; }

    void done() { mDone = true; }
    void undone() { mDone = false; }
};

} // namespace cmnd

#endif // CMND_UNDONEDELETER

