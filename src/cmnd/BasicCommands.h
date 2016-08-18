#ifndef CMND_BASICCOMMANDS_H
#define CMND_BASICCOMMANDS_H

#include <stdlib.h>
#include <string.h>
#include <functional>
#include <memory>
#include <QMap>
#include <QVector>
#include "XC.h"
#include "cmnd/Stable.h"
#include "cmnd/SleepableObject.h"

namespace cmnd
{

template <typename tObject>
class GrabNewObject : public Stable
{
    tObject* mObject;
    bool mIsCanceled;
public:
    GrabNewObject(tObject* aObject) : mObject(aObject), mIsCanceled(false) {}
    ~GrabNewObject() { if (mIsCanceled) delete mObject; }
    virtual void undo() { mIsCanceled = true; }
    virtual void redo() { mIsCanceled = false; }
};

template <typename tObject>
class GrabDeleteObject : public Stable
{
    tObject* mObject;
    bool mIsCanceled;
public:
    GrabDeleteObject(tObject* aObject) : mObject(aObject), mIsCanceled(false) {}
    ~GrabDeleteObject() { if (!mIsCanceled) delete mObject; }
    virtual void undo() { mIsCanceled = true; }
    virtual void redo() { mIsCanceled = false; }
};

class Awake : public Stable
{
    SleepableObject* mSleepable;
public:
    Awake(SleepableObject* aSleepable) : mSleepable(aSleepable) {}
    virtual void undo() { mSleepable->sleep(); }
    virtual void redo() { mSleepable->awake(); }
};

class Sleep : public Stable
{
    SleepableObject* mSleepable;
public:
    Sleep(SleepableObject* aSleepable) : mSleepable(aSleepable) {}
    virtual void undo() { mSleepable->awake(); }
    virtual void redo() { mSleepable->sleep(); }
};

class Delegatable : public Stable
{
    typedef std::function<void()> FuncType;
    FuncType mExec;
    FuncType mUndo;
    FuncType mRedo;
public:
    Delegatable(const FuncType& aExec, const FuncType& aUndo)
        : mExec(aExec)
        , mUndo(aUndo)
        , mRedo(aExec)
    {
    }

    Delegatable(const FuncType& aExec, const FuncType& aUndo, const FuncType& aRedo)
        : mExec(aExec)
        , mUndo(aUndo)
        , mRedo(aRedo)
    {
    }
    virtual void exec() { mExec(); }
    virtual void undo() { mUndo(); }
    virtual void redo() { mRedo(); }
};

template<typename tValue>
class Assign : public Stable
{
    tValue* mTarget;
    tValue mPrev;
    tValue mNext;
public:
    Assign(tValue* aTarget, tValue aValue)
        : mTarget(aTarget)
        , mPrev()
        , mNext(aValue)
    {}
    virtual void exec() { mPrev = *mTarget; redo(); }
    virtual void undo() { *mTarget = mPrev; }
    virtual void redo() { *mTarget = mNext; }
};

template<typename tTarget, typename tValue>
class ConvertAssign : public Stable
{
    tTarget* mTarget;
    tValue mPrev;
    tValue mNext;
public:
    ConvertAssign(tTarget* aTarget, tValue aValue)
        : mTarget(aTarget)
        , mPrev()
        , mNext(aValue)
    {}
    virtual void exec() { mPrev = *mTarget; redo(); }
    virtual void undo() { *mTarget = mPrev; }
    virtual void redo() { *mTarget = mNext; }
};

template<typename tTarget, typename tValue = tTarget>
class ModifiableAssign : public Stable
{
    tTarget* mTarget;
    tValue mPrev;
    tValue mNext;
    bool mDone;
public:
    ModifiableAssign(tTarget* aTarget, tValue aValue)
        : mTarget(aTarget)
        , mPrev()
        , mNext(aValue)
        , mDone(false)
    {}
    void modifyValue(tValue aValue) { mNext = aValue; if (mDone) redo(); }
    virtual void exec() { mPrev = *mTarget; redo(); }
    virtual void undo() { *mTarget = mPrev; mDone = false; }
    virtual void redo() { *mTarget = mNext; mDone = true; }
};

template<typename tObject>
class AssignNewObject : public Stable
{
    tObject** mTarget;
    tObject* mValue;

public:
    AssignNewObject(tObject** aTarget, tObject* aValue)
        : mTarget(aTarget)
        , mValue(aValue)
    {
    }
    ~AssignNewObject()
    {
        if (*mTarget != mValue) delete mValue;
    }
    virtual void undo() { *mTarget = NULL; }
    virtual void redo() { *mTarget = mValue; }
};

template<typename tVectorObj>
class PushBackVector : public Stable
{
    typedef QVector<tVectorObj> VectorType;
    VectorType* mTarget;
    tVectorObj mObj;
public:
    PushBackVector(VectorType* aTarget, tVectorObj aObj) : mTarget(aTarget), mObj(aObj) {}
    virtual void undo() { mObj = mTarget->back(); mTarget->pop_back(); }
    virtual void redo() { mTarget->push_back(mObj); }
};

template<typename tVectorObj>
class RemoveVector : public Stable
{
    typedef QVector<tVectorObj> VectorType;
    VectorType* mTarget;
    int mIndex;
    tVectorObj mObj;
public:
    RemoveVector(VectorType* aTarget, int aIndex)
        : mTarget(aTarget), mIndex(aIndex), mObj() {}
    virtual void undo() { mTarget->insert(mIndex, mObj); }
    virtual void redo() { mObj = mTarget->at(mIndex); mTarget->remove(mIndex); }
};

template<typename tListObj>
class PushBackList : public Stable
{
    typedef QList<tListObj> ListType;
    ListType* mTarget;
    tListObj mObj;
public:
    PushBackList(ListType* aTarget, tListObj aObj) : mTarget(aTarget), mObj(aObj) {}
    virtual void undo() { mObj = mTarget->back(); mTarget->pop_back(); }
    virtual void redo() { mTarget->push_back(mObj); }
};

template<typename tListObj>
class PopBackList : public Stable
{
    typedef QList<tListObj> ListType;
    ListType* mTarget;
    tListObj mObj;
public:
    PopBackList(ListType* aTarget) : mTarget(aTarget), mObj() {}
    virtual void undo()
    {
        if (mObj)
        {
            mTarget->push_back(mObj);
        }
    }
    virtual void redo()
    {
        if (!mTarget->empty())
        {
            mObj = &mTarget->back();
            mTarget->pop_back();
        }
    }
};

template<typename tListObj>
class RemoveList : public Stable
{
    typedef QList<tListObj> ListType;
    ListType* mTarget;
    int mIndex;
    tListObj mObj;
public:
    RemoveList(ListType* aTarget, int aIndex)
        : mTarget(aTarget), mIndex(aIndex), mObj() {}

    virtual void undo() { mTarget->insert(mIndex, mObj); }
    virtual void redo() { mObj = mTarget->at(mIndex); mTarget->removeAt(mIndex); }
};

template<typename tListObj>
class RemoveListByObj : public Stable
{
    typedef QList<tListObj> ListType;
    ListType* mTarget;
    int mIndex;
    tListObj mObj;
public:
    RemoveListByObj(ListType* aTarget, tListObj aObj)
        : mTarget(aTarget), mIndex(0), mObj(aObj) {}

    virtual void undo() { mTarget->insert(mIndex, mObj); }
    virtual void redo() { mIndex = mTarget->indexOf(mObj); mTarget->removeAt(mIndex); }
};

template<typename tTree>
class PushBackTree : public Stable
{
    typedef typename tTree::Children TreeChildrenType;
    TreeChildrenType* mTarget;
    tTree* mObj;
public:
    PushBackTree(TreeChildrenType* aTarget, tTree* aObj) : mTarget(aTarget), mObj(aObj) {}
    virtual void undo() { mObj = mTarget->popBack(); }
    virtual void redo() { mTarget->pushBack(mObj); mObj = NULL; }
};

template<typename tTree>
class PopBackTree : public Stable
{
    typedef typename tTree::Children TreeChildrenType;
    TreeChildrenType* mTarget;
    tTree* mObj;
public:
    PopBackTree(TreeChildrenType* aTarget)
        : mTarget(aTarget), mObj() {}
    virtual void undo() { mTarget->pushBack(mObj); }
    virtual void redo() { mObj = mTarget->popBack(); }
};

template<typename tTree>
class PushBackNewTreeObject : public Stable
{
    typedef typename tTree::Children TreeChildrenType;
    TreeChildrenType* mTarget;
    tTree* mObj;

public:
    PushBackNewTreeObject(TreeChildrenType* aTarget, tTree* aObj)
        : mTarget(aTarget)
        , mObj(aObj)
    {
    }
    ~PushBackNewTreeObject()
    {
        if (mObj != NULL) delete mObj;
    }
    virtual void undo()
    {
        mObj = mTarget->popBack();
    }
    virtual void redo()
    {
        mTarget->pushBack(mObj);
        mObj = NULL;
    }
};

template<typename tTree>
class InsertTree : public Stable
{
    typedef typename tTree::Children ListType;
    ListType* mTarget;
    int mIndex;
    tTree* mObj;

public:
    InsertTree(ListType* aTarget, int aIndex, tTree* aObj)
        : mTarget(aTarget), mIndex(aIndex), mObj(aObj) {}

    virtual void undo() { mTarget->erase(mIndex); }
    virtual void redo() { mTarget->insert(mIndex, mObj); }
};

template<typename tTree>
class RemoveTree : public Stable
{
    typedef typename tTree::Children ListType;
    ListType* mTarget;
    int mIndex;
    tTree* mObj;

public:
    RemoveTree(ListType* aTarget, int aIndex)
        : mTarget(aTarget), mIndex(aIndex), mObj() {}

    virtual void undo() { mTarget->insert(mIndex, mObj); }
    virtual void redo() { mObj = *(mTarget->at(mIndex)); mTarget->erase(mIndex); }
};

template<typename tTree>
class RemoveTreeByObj : public Stable
{
    typedef typename tTree::Children ListType;
    ListType* mTarget;
    int mIndex;
    tTree* mObj;

public:
    RemoveTreeByObj(ListType* aTarget, tTree* aObj)
        : mTarget(aTarget), mIndex(-1), mObj(aObj) {}

    virtual void undo() { mTarget->insert(mIndex, mObj); }
    virtual void redo()
    {
        mIndex = mTarget->indexOf(mObj);
        XC_ASSERT(mIndex >= 0);
        mTarget->erase(mIndex);
    }
};

template <typename tKey, typename tValue>
class InsertMap : public Stable
{
    QMap<tKey, tValue>& mMap;
    tKey mKey;
    tValue mValue;
public:
    InsertMap(QMap<tKey, tValue>& aMap, tKey aKey, tValue aValue)
        : mMap(aMap), mKey(aKey), mValue(aValue)
    {}

    virtual void undo()
    {
        XC_ASSERT(mMap.contains(mKey));
        const int removeCount = mMap.remove(mKey);
        XC_ASSERT(removeCount == 1); (void)removeCount;
    }

    virtual void redo()
    {
        XC_ASSERT(!mMap.contains(mKey));
        mMap.insert(mKey, mValue);
    }
};

template <typename tKey, typename tValue>
class RemoveMap : public Stable
{
    QMap<tKey, tValue>& mMap;
    tKey mKey;
    tValue mPrev;
public:
    RemoveMap(QMap<tKey, tValue>& aMap, tKey aKey)
        : mMap(aMap), mKey(aKey), mPrev()
    {}

    virtual void undo()
    {
        XC_ASSERT(!mMap.contains(mKey));
        mMap.insert(mKey, mPrev);
    }

    virtual void redo()
    {
        XC_ASSERT(mMap.contains(mKey));
        mPrev = mMap[mKey];
        const int removeCount = mMap.remove(mKey);
        XC_ASSERT(removeCount == 1); (void)removeCount;
    }
};

class AssignMemory : public Stable
{
    void* mTarget;
    size_t mSize;
    std::unique_ptr<uint8[]> mAssign;
    std::unique_ptr<uint8[]> mCopyBlock;
    size_t mCopySize;
    bool mDone;

    bool copiesOneTime() const { return mCopySize == mSize; }
    uint8* assignData() { return mAssign.get(); }

    void exchange(uint8* aBuffer0, uint8* aBuffer1)
    {
        const uint8* block = mCopyBlock.get();
        uint8* buffer0 = aBuffer0;
        uint8* buffer1 = aBuffer1;
        size_t size    = mSize;

        while (size > 0)
        {
            const size_t copySize = std::min(mCopySize, size);
            memcpy((void*)block,   (void*)buffer0, copySize);
            memcpy((void*)buffer0, (void*)buffer1, copySize);
            memcpy((void*)buffer1, (void*)block,   copySize);
            buffer0 += copySize;
            buffer1 += copySize;
            size    -= copySize;
        }
    }

public:
    AssignMemory(void* aTarget, const void* aAssign, size_t aSize, size_t aCopySize = 0)
        : mTarget(aTarget)
        , mSize(aSize)
        , mAssign()
        , mCopyBlock()
        , mCopySize(aCopySize > 0 ? aCopySize : aSize)
        , mDone(false)
    {
        XC_ASSERT(aSize > 0);

        if (mCopySize < mSize)
        {
            mCopyBlock.reset(new uint8[mCopySize]);
        }
        else
        {
            mCopySize = mSize;
            mCopyBlock.reset(new uint8[mSize]);
        }

        mAssign.reset(new uint8[mSize]);
        memcpy(mAssign.get(), aAssign, mSize);
    }

    AssignMemory(void* aTarget, std::unique_ptr<uint8[]>& aMovedAssign, size_t aSize, size_t aCopySize = 0)
        : mTarget(aTarget)
        , mSize(aSize)
        , mAssign(std::move(aMovedAssign))
        , mCopyBlock()
        , mCopySize(aCopySize > 0 ? aCopySize : aSize)
        , mDone(false)
    {
        XC_ASSERT(aSize > 0);
        XC_ASSERT(aCopySize > 0);

        if (mCopySize < mSize)
        {
            mCopyBlock.reset(new uint8[mCopySize]);
        }
        else
        {
            mCopySize = mSize;
            mCopyBlock.reset(new uint8[mSize]);
        }
    }

    const void* target() const { return mTarget; }
    size_t size() const { return mSize; }

    void modifyValue(const void* aNewAssign)
    {
        if (copiesOneTime() || !mDone)
        {
            memcpy(assignData(), aNewAssign, mSize);
        }
        if (mDone)
        {
            memcpy(mTarget, aNewAssign, mSize);
        }
    }

    virtual void exec()
    {
        if (copiesOneTime())
        {
            memcpy(mCopyBlock.get(), mTarget, mSize);
        }
        redo();
    }

    virtual void undo()
    {
        if (copiesOneTime())
        {
            memcpy(mTarget, mCopyBlock.get(), mSize);
        }
        else
        {
            exchange((uint8*)mTarget, assignData());
        }

        mDone = false;
    }

    virtual void redo()
    {
        if (copiesOneTime())
        {
            memcpy(mTarget, assignData(), mSize);
        }
        else
        {
            exchange((uint8*)mTarget, assignData());
        }

        mDone = true;
    }
};

class DebugDumper : public Stable
{
    QString mText;
public:
    DebugDumper(const QString& aText) : mText(aText) { qDebug() << "cnst :" << mText; }
    ~DebugDumper() { qDebug() << "dest :" << mText; }
    virtual void exec() { qDebug() << "exec :" << mText; }
    virtual void redo() { qDebug() << "redo :" << mText; }
    virtual void undo() { qDebug() << "undo :" << mText; }
};

} // namespace cmnd

#endif // CMND_BASICCOMMANDS_H
