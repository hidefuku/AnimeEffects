#ifndef CTRL_POSEKEY_H
#define CTRL_POSEKEY_H

#include <memory>
#include <QList>
#include "util/LinkPointer.h"
#include "util/Easing.h"
#include "core/TimeKey.h"
#include "core/Bone2.h"
namespace core { class BoneKey; }

namespace core
{

class PoseKey : public TimeKey
{
public:
    class Data
    {
        util::Easing::Param mEasing;
        QList<Bone2*> mTopBones;
    public:
        Data();
        Data(const Data& aRhs);
        Data& operator=(const Data& aRhs);
        ~Data();

        util::Easing::Param& easing() { return mEasing; }
        const util::Easing::Param& easing() const { return mEasing; }

        void createBonesBy(BoneKey& aAreaBone);

        QList<Bone2*>& topBones();
        const QList<Bone2*>& topBones() const;

        bool isEmpty() const { return mTopBones.empty(); }
        void deleteAll();
    };

    PoseKey();

    Data& data() { return mData; }
    const Data& data() const { return mData; }

    virtual TimeKeyType type() const { return TimeKeyType_Pose; }
    virtual TimeKey* createClone();
    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

private:
    bool serializeBone(Serializer& aOut, const Bone2* aBone) const;
    bool deserializeBone(Deserializer& aIn, Bone2* aBone);

    Data mData;
};

} // namespace core

#endif // CTRL_POSEKEY_H
