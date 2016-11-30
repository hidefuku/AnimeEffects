#ifndef CORE_TIMELINE_H
#define CORE_TIMELINE_H

#include <array>
#include <QMap>
#include <QScopedPointer>
#include "cmnd/Vector.h"
#include "core/TimeKey.h"
#include "core/TimeKeyType.h"
#include "core/Serializer.h"
#include "core/Deserializer.h"
namespace core { class Project; }
namespace core { class ObjectNode; }
namespace core { class TimeKeyExpans; }


namespace core
{

class TimeLine
{
public:
    enum { kDefaultKeyIndex = -1 };

    typedef QMap<int, TimeKey*> MapType;

    static QString getTimeKeyName(TimeKeyType aType);
    static TimeKeyType getTimeKeyType(const QString& aName);
    static TimeKeyType getTimeKeyTypeInOrderOfOperations(int aIndex);

    TimeLine();
    ~TimeLine();

    bool hasTimeKey(int aIndex) const;
    bool hasTimeKey(TimeKeyType aType, int aIndex) const;
    bool isEmpty() const;
    bool isEmpty(TimeKeyType aType) const;
    int validTypeCount() const;

    const MapType& map(TimeKeyType aType) const;

    TimeKey* timeKey(TimeKeyType aType, int aIndex);
    const TimeKey* timeKey(TimeKeyType aType, int aIndex) const;

    void grabDefaultKey(TimeKeyType aType, TimeKey* aKey);
    TimeKey* defaultKey(TimeKeyType aType);
    const TimeKey* defaultKey(TimeKeyType aType) const;

    TimeKeyExpans& current() { return *mCurrent; }
    const TimeKeyExpans& current() const { return *mCurrent; }

    TimeKeyExpans& working() { return *mWorking; }
    const TimeKeyExpans& working() const { return *mWorking; }

    bool move(TimeKeyType aType, int aFrom, int aTo);
    cmnd::Base* createPusher(TimeKeyType aType, int aFrame, TimeKey* aTimeKey);
    cmnd::Base* createRemover(TimeKeyType aType, int aFrame, bool aOptional = false);

    bool serialize(Serializer& aOut) const;
    bool deserialize(Deserializer& aIn);

private:
    void clear();
    int serializeTypeCount() const;
    void pushRemoveCommands(
            TimeKeyType aType, int aFrame, cmnd::Vector& aCommands);
    bool serializeTimeKey(Serializer& aOut, const TimeKey& aTimeKey) const;
    bool deserializeTimeKey(Deserializer& aIn, TimeKeyType aType, int aIndex);

    std::array<MapType, TimeKeyType_TERM> mMap;
    QScopedPointer<TimeKeyExpans> mCurrent;
    QScopedPointer<TimeKeyExpans> mWorking;
    std::array<QScopedPointer<TimeKey>, TimeKeyType_TERM> mDefaultKeys;
};

} // namespace core

#endif // CORE_TIMELINE_H
