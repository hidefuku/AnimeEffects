#ifndef CORE_TIMEKEY_H
#define CORE_TIMEKEY_H

#include "util/LifeLink.h"
#include "util/TreeNodeBase.h"
#include "util/TreeIterator.h"
#include "util/NonCopyable.h"
#include "cmnd/SleepableObject.h"
#include "core/Serializer.h"
#include "core/Deserializer.h"
#include "core/TimeKeyType.h"

namespace core
{

class TimeKey
        : public util::TreeNodeBase<TimeKey>
        , public cmnd::SleepableObject
        , private util::NonCopyable
{
public:
    typedef util::TreeNodeBase<TimeKey>::Children ChildrenType;
    typedef util::TreeIterator<TimeKey, ChildrenType::Iterator> Iterator;
    typedef util::TreeIterator<const TimeKey, ChildrenType::ConstIterator> ConstIterator;

    TimeKey();

    virtual ~TimeKey() {}

    virtual TimeKeyType type() const = 0;
    virtual bool canHoldChild() const { return false; }

    virtual void sleep() {}
    virtual void awake() {}

    int frame() const { return mFrame; }
    void setFrame(int aFrame) { mFrame = aFrame; }

    bool isDefaultKey() const { return mFrame == -1; }

    bool isFocused() const { return mFocus.isLinking(); }
    void setFocus(util::LifeLink& aLink) { mFocus = aLink; }

    bool isSelected() const { return mSelect.isLinking(); }
    void setSelect(util::LifeLink& aLink) { mSelect = aLink; }

    virtual TimeKey* createClone() = 0;

    virtual bool serialize(Serializer& aOut) const = 0;
    virtual bool deserialize(Deserializer& aIn) = 0;

private:
    util::LifeLink::Node mFocus;
    util::LifeLink::Node mSelect;
    int mFrame;
};

} // namespace core


#define TIMEKEY_TYPE_ASSERT(aKey, aType) \
    XC_MSG_ASSERT((aKey).type() == core::TimeKeyType_##aType, "timekey type error: %d", (aKey).type())

#define TIMEKEY_PTR_TYPE_ASSERT(aKeyPtr, aType) \
    do { XC_PTR_ASSERT(aKeyPtr); TIMEKEY_TYPE_ASSERT(*(aKeyPtr), aType); } while(0)


#endif // CORE_TIMEKEY_H
