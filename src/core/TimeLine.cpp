#include "XC.h"
#include "cmnd/BasicCommands.h"
#include "cmnd/Scalable.h"
#include "core/TimeLine.h"
#include "core/TimeKeyExpans.h"
#include "core/Project.h"

namespace
{

static const int kMaxLengthOfTimeKeyName = 8;
static const std::array<const char*, core::TimeKeyType_TERM> kTimeKeyNames = {
    "SRT",
    "Opa",
    "Bone",
    "Pose",
    "Mesh",
    "FFD"
};

QString getTimeKeyName(core::TimeKeyType aType)
{
    return QString(kTimeKeyNames.at(aType));
}

core::TimeKeyType getTimeKeyType(const QString& aName)
{
    for (int i = 0; i < core::TimeKeyType_TERM; ++i)
    {
        if (getTimeKeyName((core::TimeKeyType)i) == aName)
        {
            return (core::TimeKeyType)i;
        }
    }
    return core::TimeKeyType_TERM;
}

}

namespace core
{

//---------------------------------------------------------------------------------------
QString TimeLine::timeKeyName(TimeKeyType aType)
{
    return getTimeKeyName(aType);
}

TimeLine::TimeLine()
    : mMap()
    , mCurrent(new TimeKeyExpans())
    , mWorking(new TimeKeyExpans())
{
}

TimeLine::~TimeLine()
{
    clear();
}

bool TimeLine::hasTimeKey(int aIndex) const
{
    for (int i = 0; i < TimeKeyType_TERM; ++i)
    {
        if (mMap[i].contains(aIndex)) return true;
    }
    return false;
}

bool TimeLine::hasTimeKey(TimeKeyType aType, int aIndex) const
{
    return mMap.at(aType).contains(aIndex);
}

bool TimeLine::isEmpty() const
{
    for (int i = 0; i < TimeKeyType_TERM; ++i)
    {
        if (!mMap[i].isEmpty()) return false;
    }
    return true;
}

bool TimeLine::isEmpty(TimeKeyType aType) const
{
    return mMap.at(aType).isEmpty();
}

int TimeLine::validTypeCount() const
{
    int count = 0;
    for (int i = 0; i < TimeKeyType_TERM; ++i)
    {
        if (!mMap[i].isEmpty()) ++count;
    }
    return count;
}

const TimeLine::MapType& TimeLine::map(TimeKeyType aType) const
{
    return mMap.at(aType);
}

TimeKey* TimeLine::timeKey(TimeKeyType aType, int aIndex)
{
    if (mMap.at(aType).contains(aIndex))
    {
        return mMap[aType][aIndex];
    }
    return NULL;
}

const TimeKey* TimeLine::timeKey(TimeKeyType aType, int aIndex) const
{
    if (mMap.at(aType).contains(aIndex))
    {
        return mMap[aType][aIndex];
    }
    return NULL;
}

void TimeLine::clear()
{
    for (int i = 0; i < TimeKeyType_TERM; ++i)
    {
        QList<TimeKey*> values = mMap[i].values();
        qDeleteAll(values.begin(), values.end());
        mMap[i].clear();
    }
    mCurrent.reset(new TimeKeyExpans());
}

bool TimeLine::move(TimeKeyType aType, int aFrom, int aTo)
{
    TimeLine::MapType& map = mMap.at(aType);

    if (!map.contains(aFrom) || map.contains(aTo))
    {
        return false;
    }

    TimeKey* key = map.value(aFrom);
    XC_PTR_ASSERT(key);
    map.remove(aFrom);
    map.insert(aTo, key);
    key->setFrame(aTo);
    return true;
}

cmnd::Base* TimeLine::createPusher(TimeKeyType aType, int aFrame, TimeKey* aTimeKey)
{
    return new cmnd::LambdaScalable([=](cmnd::Vector& aCommands)
    {
        TimeLine::MapType& map = this->mMap.at(aType);
        // remove old key
        if (map.contains(aFrame))
        {
            pushRemoveCommands(aType, aFrame, aCommands);
        }
        aTimeKey->setFrame(aFrame);
        aCommands.push(new cmnd::InsertMap<int, TimeKey*>(map, aFrame, aTimeKey));
    });
}

cmnd::Base* TimeLine::createRemover(TimeKeyType aType, int aFrame, bool aOptional)
{
    return new cmnd::LambdaScalable([=](cmnd::Vector& aCommands)
    {
        TimeLine::MapType& map = this->mMap.at(aType);
        XC_ASSERT(aOptional || map.contains(aFrame)); (void)aOptional;

        if (map.contains(aFrame))
        {
            // remove
            pushRemoveCommands(aType, aFrame, aCommands);
        }
    });
}

void TimeLine::pushRemoveCommands(
        TimeKeyType aType, int aFrame, cmnd::Vector& aCommands)
{
    TimeLine::MapType& map = mMap.at(aType);
    TimeKey* key = map.value(aFrame);
    XC_ASSERT(key->frame() == aFrame);

    // remove children
    for (auto itr = key->children().rbegin(); itr != key->children().rend(); ++itr)
    {
        TimeKey* child = *itr;
        TimeLine::MapType& cmap = mMap.at(child->type());
        const int cframe = child->frame();
        XC_ASSERT(child == cmap.value(cframe));
        aCommands.push(new cmnd::PopBackTree<TimeKey>(&key->children()));
        aCommands.push(new cmnd::RemoveMap<int, TimeKey*>(cmap, cframe));
        aCommands.push(new cmnd::GrabDeleteObject<TimeKey>(child));
    }

    // remove from parent
    if (key->parent())
    {
        aCommands.push(new cmnd::RemoveTreeByObj<TimeKey>(&key->parent()->children(), key));
    }

    // remove
    aCommands.push(new cmnd::RemoveMap<int, TimeKey*>(map, aFrame));
    aCommands.push(new cmnd::GrabDeleteObject<TimeKey>(key));
}

bool TimeLine::serialize(Serializer& aOut) const
{
    static const std::array<uint8, 8> kSignature =
        { 'T', 'i', 'm', 'e', 'L', 'i', 'n', 'e' };
    static const std::array<uint8, 8> kMapSignature =
        { 'T', 'i', 'm', 'e', 'M', 'a', 'p', '_' };

    // signature
    auto pos = aOut.beginBlock(kSignature);

    // valid map count
    aOut.write(validTypeCount());

    // each map
    for (int i = 0; i < TimeKeyType_TERM; ++i)
    {
        const MapType& map = mMap[i];

        if (map.isEmpty()) continue;

        // signature
        auto mapPos = aOut.beginBlock(kMapSignature);

        // type name
        aOut.write(getTimeKeyName((TimeKeyType)i));

        // key count
        aOut.write(map.count());

        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            // timekey index
            aOut.write(itr.key());

            const TimeKey* timeKey = itr.value();
            XC_PTR_ASSERT(timeKey);

            // reference id
            aOut.writeID(timeKey);

            // timekey value
            if (!timeKey->serialize(aOut))
            {
                return false;
            }

            // child count
            aOut.write((int)timeKey->children().size());

            // references to children
            for (const TimeKey* child : timeKey->children())
            {
                aOut.writeID(child);
            }
        }

        aOut.endBlock(mapPos);
    }

    aOut.endBlock(pos);

    return aOut.checkStream();
}

bool TimeLine::deserialize(Deserializer& aIn)
{
    clear();

    // check block begin
    if (!aIn.beginBlock("TimeLine"))
    {
        return false;
    }
    aIn.pushLogScope("TimeLine");

    // valid map count
    int mapCount = 0;
    aIn.read(mapCount);

    // each map
    for (int i = 0; i < mapCount; ++i)
    {
        // check block begin
        if (!aIn.beginBlock("TimeMap_"))
        {
            return false;
        }
        aIn.pushLogScope("TimeKeyMap");

        // key type
        QString typeName;
        aIn.read(typeName, kMaxLengthOfTimeKeyName);
        auto typeIndex = getTimeKeyType(typeName);

        // pass only supported index
        if (typeIndex >= TimeKeyType_TERM) continue;

        MapType& map = mMap[typeIndex];

        // timekey count
        int keyCount = 0;
        aIn.read(keyCount);

        for (int k = 0; k < keyCount; ++k)
        {
            // timekey index
            int keyIndex = 0;
            aIn.read(keyIndex);

            TimeKey* key = nullptr;
            if (typeIndex == TimeKeyType_SRT)
            {
                key = new SRTKey();
            }
            else if (typeIndex == TimeKeyType_Opa)
            {
                key = new OpaKey();
            }
            else if (typeIndex == TimeKeyType_Bone)
            {
                key = new BoneKey();
            }
            else if (typeIndex == TimeKeyType_Pose)
            {
                key = new PoseKey();
            }
            else if (typeIndex == TimeKeyType_Mesh)
            {
                key = new MeshKey();
            }
            else if (typeIndex == TimeKeyType_FFD)
            {
                key = new FFDKey();
            }

            // reference id
            if (!aIn.bindIDData(key))
            {
                return aIn.errored("failed to bind reference id");
            }

            // set frame
            key->setFrame(keyIndex);

            // timekey value
            if (!key->deserialize(aIn))
            {
                delete key;
                return false;
            }
            map.insert(keyIndex, key);

            // child count
            int childCount = 0;
            aIn.read(childCount);

            // references to children
            for (int ci = 0; ci < childCount; ++ci)
            {
                auto solver = [=](void* aPtr) {
                    TimeKey* child = static_cast<TimeKey*>(aPtr);
                    key->children().pushBack(child);
                };
                if (!aIn.orderIDData(solver))
                {
                    return aIn.errored("invalid child reference id");
                }
            }
        }

        // check block end
        if (!aIn.endBlock())
        {
            return false;
        }
        aIn.popLogScope();
    }

    // check block end
    if (!aIn.endBlock())
    {
        return false;
    }
    aIn.popLogScope();

    return aIn.checkStream();
}

} // namespace core
