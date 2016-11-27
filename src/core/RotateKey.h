#ifndef CORE_ROTATEKEY_H
#define CORE_ROTATEKEY_H

#include "util/Easing.h"
#include "core/TimeKey.h"

namespace core
{

class RotateKey : public TimeKey
{
public:
    struct Data
    {
        util::Easing::Param easing;
        float rotate;
        Data();
        void clamp();
    };

    RotateKey();

    void setRotate(float aRotate) { mData.rotate = aRotate; }
    const float& rotate() const { return mData.rotate; }

    Data& data() { return mData; }
    const Data& data() const { return mData; }

    virtual TimeKeyType type() const { return TimeKeyType_Rotate; }
    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

private:
    Data mData;
};

} // namespace core

#endif // CORE_ROTATEKEY_H
