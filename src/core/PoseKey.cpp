#include "util/TreeUtil.h"
#include "core/PoseKey.h"
#include "core/BoneKey.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
PoseKey::Data::Data()
    : mEasing()
    , mTopBones()
{
}

PoseKey::Data::Data(const Data& aRhs)
{
    mEasing = aRhs.easing();

    for (const Bone2* bone : aRhs.topBones())
    {
        mTopBones.push_back(util::TreeUtil::createClone(bone));
    }
}

PoseKey::Data& PoseKey::Data::operator=(const Data& aRhs)
{
    deleteAll();
    mEasing = aRhs.easing();

    for (const Bone2* bone : aRhs.topBones())
    {
        mTopBones.push_back(util::TreeUtil::createClone(bone));
    }
    return *this;
}

PoseKey::Data::~Data()
{
    deleteAll();
}

void PoseKey::Data::createBonesBy(BoneKey& aAreaBone)
{
    deleteAll();
    for (Bone2* bone : aAreaBone.data().topBones())
    {
        mTopBones.push_back(bone->createShadow());
    }
}

QList<Bone2*>& PoseKey::Data::topBones()
{
    return mTopBones;
}

const QList<Bone2*>& PoseKey::Data::topBones() const
{
    return mTopBones;
}

void PoseKey::Data::deleteAll()
{
    qDeleteAll(mTopBones);
    mTopBones.clear();
}

//-------------------------------------------------------------------------------------------------
PoseKey::PoseKey()
    : mData()
{
}

TimeKey* PoseKey::createClone()
{
    auto newKey = new PoseKey();
    newKey->mData = this->mData;
    return newKey;
}

bool PoseKey::serialize(Serializer& aOut) const
{
    // easing
    aOut.write(mData.easing());

    // top bone count
    aOut.write(mData.topBones().count());

    // serialize all bones
    for (auto topBone : mData.topBones())
    {
        XC_PTR_ASSERT(topBone);
        if (!serializeBone(aOut, topBone))
        {
            return false;
        }
    }

    return aOut.checkStream();
}

bool PoseKey::serializeBone(Serializer& aOut, const Bone2* aBone) const
{
    if (!aBone) return true;

    // child count
    aOut.write((int)aBone->children().size());

    // serialize bone
    if (!aBone->serialize(aOut))
    {
        return false;
    }

    // iterate children
    for (auto child : aBone->children())
    {
        if (!serializeBone(aOut, child))
        {
            return false;
        }
    }

    return aOut.checkStream();
}

bool PoseKey::deserialize(Deserializer& aIn)
{
    mData.deleteAll();

    aIn.pushLogScope("PoseKey");

    if (!aIn.read(mData.easing()))
    {
        return aIn.errored("invalid easing param");
    }

    // top bone count
    int topBoneCount = 0;
    aIn.read(topBoneCount);
    if (topBoneCount < 0)
    {
        return aIn.errored("invalid top bone count");
    }

    // deserialize all bones
    for (int i = 0; i < topBoneCount; ++i)
    {
        Bone2* topBone = new Bone2();
        mData.topBones().push_back(topBone);

        if (!deserializeBone(aIn, topBone))
        {
            return false;
        }
    }

    aIn.popLogScope();
    return aIn.checkStream();
}

bool PoseKey::deserializeBone(Deserializer& aIn, Bone2* aBone)
{
    if (!aBone) return true;

    // child count
    int childCount = 0;
    aIn.read(childCount);
    if (childCount < 0)
    {
        return aIn.errored("invalid child count");
    }

    // deserialize bone
    if (!aBone->deserialize(aIn))
    {
        return false;
    }

    // iterate children
    for (int i = 0; i < childCount; ++i)
    {
        Bone2* child = new Bone2();
        aBone->children().pushBack(child);

        if (!deserializeBone(aIn, child))
        {
            return false;
        }
    }

    return aIn.checkStream();
}

} // namespace core
