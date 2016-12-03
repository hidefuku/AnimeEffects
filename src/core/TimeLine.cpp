#include "XC.h"
#include "cmnd/BasicCommands.h"
#include "cmnd/Scalable.h"
#include "core/TimeLine.h"
#include "core/TimeKeyExpans.h"
#include "core/DepthKey.h"
#include "core/Project.h"

namespace
{

static const int kMaxLengthOfTimeKeyName = 8;
static const std::array<const char*, core::TimeKeyType_TERM> kTimeKeyNames = {
    "Move",
    "Rotate",
    "Scale",
    "Depth",
    "Opa",
    "Bone",
    "Pose",
    "Mesh",
    "FFD",
    "Image"
};

}

namespace core
{

//---------------------------------------------------------------------------------------
QString TimeLine::getTimeKeyName(TimeKeyType aType)
{
    return QString(kTimeKeyNames.at(aType));
}

TimeKeyType TimeLine::getTimeKeyType(const QString& aName)
{
    for (int i = 0; i < TimeKeyType_TERM; ++i)
    {
        if (getTimeKeyName((TimeKeyType)i) == aName)
        {
            return (TimeKeyType)i;
        }
    }
    return core::TimeKeyType_TERM;
}

TimeKeyType TimeLine::getTimeKeyTypeInOrderOfOperations(int aIndex)
{
    switch (aIndex)
    {
    case 0: return TimeKeyType_Image;
    case 1: return TimeKeyType_Mesh;
    case 2: return TimeKeyType_FFD;
    case 3: return TimeKeyType_Bone;
    case 4: return TimeKeyType_Pose;
    case 5: return TimeKeyType_Move;
    case 6: return TimeKeyType_Rotate;
    case 7: return TimeKeyType_Scale;
    case 8: return TimeKeyType_Depth;
    case 9: return TimeKeyType_Opa;
    default: XC_ASSERT(0); return TimeKeyType_TERM;
    }
}

TimeLine::TimeLine()
    : mMap()
    , mCurrent(new TimeKeyExpans())
    , mWorking(new TimeKeyExpans())
    , mDefaultKeys()
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

void TimeLine::grabDefaultKey(TimeKeyType aType, TimeKey* aKey)
{
    if (aKey)
    {
        aKey->setFrame(kDefaultKeyIndex);
    }
    mDefaultKeys.at(aType).reset(aKey);
}

TimeKey* TimeLine::defaultKey(TimeKeyType aType)
{
    return mDefaultKeys.at(aType).data();
}

const TimeKey* TimeLine::defaultKey(TimeKeyType aType) const
{
    return mDefaultKeys.at(aType).data();
}

void TimeLine::clear()
{
    for (int i = 0; i < TimeKeyType_TERM; ++i)
    {
        mDefaultKeys[i].reset();

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
    XC_ASSERT(aFrame >= 0);
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
    XC_ASSERT(aFrame >= 0);
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
        aCommands.push(new cmnd::Sleep(child));
        aCommands.push(new cmnd::GrabDeleteObject<TimeKey>(child));
    }

    // remove from parent
    if (key->parent())
    {
        aCommands.push(new cmnd::RemoveTreeByObj<TimeKey>(&key->parent()->children(), key));
    }

    // remove
    aCommands.push(new cmnd::RemoveMap<int, TimeKey*>(map, aFrame));
    aCommands.push(new cmnd::Sleep(key));
    aCommands.push(new cmnd::GrabDeleteObject<TimeKey>(key));
}

int TimeLine::serializeTypeCount() const
{
    int count = 0;
    for (int i = 0; i < TimeKeyType_TERM; ++i)
    {
        if (mDefaultKeys[i] || !mMap[i].isEmpty()) ++count;
    }
    return count;
}

bool TimeLine::serialize(Serializer& aOut) const
{
    static const std::array<uint8, 8> kSignature =
        { 'T', 'i', 'm', 'e', 'L', 'i', 'n', 'e' };
    static const std::array<uint8, 8> kMapSignature =
        { 'T', 'i', 'm', 'e', 'M', 'a', 'p', '_' };

    // signature
    auto pos = aOut.beginBlock(kSignature);

    // non empty type count (including default keys)
    aOut.write(serializeTypeCount());

    // each map
    for (int i = 0; i < TimeKeyType_TERM; ++i)
    {
        const MapType& map = mMap[i];
        auto defaultKey = mDefaultKeys[i].data();

        if (!defaultKey && map.isEmpty()) continue;

        // signature
        auto mapPos = aOut.beginBlock(kMapSignature);

        // type name
        aOut.write(getTimeKeyName((TimeKeyType)i));

        // default key is exists
        aOut.write((bool)defaultKey);

        // defautltKey
        if (defaultKey)
        {
            if (!serializeTimeKey(aOut, *defaultKey))
            {
                return false;
            }
        }

        // key count
        aOut.write(map.count());

        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            // timekey index
            aOut.write(itr.key());

            const TimeKey* timeKey = itr.value();
            XC_PTR_ASSERT(timeKey);

            // time key
            if (!serializeTimeKey(aOut, *timeKey))
            {
                return false;
            }
        }

        aOut.endBlock(mapPos);
    }

    aOut.endBlock(pos);

    return aOut.checkStream();
}

bool TimeLine::serializeTimeKey(Serializer& aOut, const TimeKey& aTimeKey) const
{
    // reference id
    aOut.writeID(&aTimeKey);

    // timekey value
    if (!aTimeKey.serialize(aOut))
    {
        return false;
    }

    // child count
    aOut.write((int)aTimeKey.children().size());

    // references to children
    for (const TimeKey* child : aTimeKey.children())
    {
        aOut.writeID(child);
    }
    return true;
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

    // non empty type count (including default keys)
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

        // default key is exists
        bool hasDefaultKey = false;
        aIn.read(hasDefaultKey);

        // default key
        if (hasDefaultKey)
        {
            if (!deserializeTimeKey(aIn, (TimeKeyType)typeIndex, -1))
            {
                return false;
            }
        }

        // timekey count
        int keyCount = 0;
        aIn.read(keyCount);

        for (int k = 0; k < keyCount; ++k)
        {
            // timekey index
            int keyIndex = 0;
            aIn.read(keyIndex);

            // time key
            if (!deserializeTimeKey(aIn, (TimeKeyType)typeIndex, keyIndex))
            {
                return false;
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

bool TimeLine::deserializeTimeKey(Deserializer& aIn, TimeKeyType aType, int aIndex)
{
    TimeKey* key = nullptr;
    if (aType == TimeKeyType_Move)
    {
        key = new MoveKey();
    }
    else if (aType == TimeKeyType_Rotate)
    {
        key = new RotateKey();
    }
    else if (aType == TimeKeyType_Scale)
    {
        key = new ScaleKey();
    }
    else if (aType == TimeKeyType_Depth)
    {
        key = new DepthKey();
    }
    else if (aType == TimeKeyType_Opa)
    {
        key = new OpaKey();
    }
    else if (aType == TimeKeyType_Bone)
    {
        key = new BoneKey();
    }
    else if (aType == TimeKeyType_Pose)
    {
        key = new PoseKey();
    }
    else if (aType == TimeKeyType_Mesh)
    {
        key = new MeshKey();
    }
    else if (aType == TimeKeyType_FFD)
    {
        key = new FFDKey();
    }
    else if (aType == TimeKeyType_Image)
    {
        key = new ImageKey();
    }

    // reference id
    if (!aIn.bindIDData(key))
    {
        return aIn.errored("failed to bind reference id");
    }

    // set frame
    key->setFrame(aIndex);

    // timekey value
    if (!key->deserialize(aIn))
    {
        delete key;
        return false;
    }

    if (aIndex == -1)
    {
        mDefaultKeys[aType].reset(key);
    }
    else
    {
        mMap[aType].insert(aIndex, key);
    }

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
    return true;
}

} // namespace core
