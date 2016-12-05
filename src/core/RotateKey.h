#ifndef CORE_ROTATEKEY_H
#define CORE_ROTATEKEY_H

#include "util/Easing.h"
#include "core/TimeKey.h"

namespace core
{

class RotateKey : public TimeKey
{
public:
    class Data
    {
        util::Easing::Param mEasing;
        float mRotate;
        void clamp();
    public:
        Data();
        util::Easing::Param& easing() { return mEasing; }
        const util::Easing::Param& easing() const { return mEasing; }
        void setRotate(float aRotate) { mRotate = aRotate; clamp(); }
        void addRotate(float aAdd) { mRotate += aAdd; clamp(); }
        const float& rotate() const { return mRotate; }
    };

    RotateKey();

    void setRotate(float aRotate) { mData.setRotate(aRotate); }
    const float& rotate() const { return mData.rotate(); }

    Data& data() { return mData; }
    const Data& data() const { return mData; }

    virtual TimeKeyType type() const { return TimeKeyType_Rotate; }
    virtual TimeKey* createClone();
    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

private:
    Data mData;
};

} // namespace core

#endif // CORE_ROTATEKEY_H
