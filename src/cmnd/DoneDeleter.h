#ifndef CMND_DONEDELETER
#define CMND_DONEDELETER

#include "util/NonCopyable.h"

namespace cmnd
{

template<typename tObj>
class DoneDeleter : private util::NonCopyable
{
    tObj* mObj;
    bool mDone;

public:
    DoneDeleter()
        : mObj()
        , mDone()
    {}

    explicit DoneDeleter(tObj* aObj)
        : mObj(aObj)
        , mDone()
    {}

    ~DoneDeleter()
    {
        if (mObj && mDone) { delete mObj; }
    }

    void set(tObj* aObj) { mObj = aObj; }
    tObj* get() const { return mObj; }
    tObj* operator->() const { return mObj; }
    explicit operator bool() const { return mObj != nullptr; }
    bool operator==(const tObj* aRhs) { return mObj == aRhs; }
    bool operator==(const DoneDeleter<tObj>& aRhs) { return mObj == aRhs.mObj; }

    void done() { mDone = true; }
    void undone() { mDone = false; }
};

} // namespace cmnd

#endif // CMND_DONEDELETER

