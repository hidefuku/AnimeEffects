#ifndef CORE_OPAKEY_H
#define CORE_OPAKEY_H

#include "util/Easing.h"
#include "core/TimeKey.h"

namespace core
{

class OpaKey : public TimeKey
{
public:
    class Data
    {
        util::Easing::Param mEasing;
        float mOpacity;
        void clamp();
    public:
        Data();

        util::Easing::Param& easing() { return mEasing; }
        const util::Easing::Param& easing() const { return mEasing; }

        void setOpacity(float aOpacity) { mOpacity = aOpacity; clamp(); }
        const float& opacity() const { return mOpacity; }

        bool isZero() const;
    };

    OpaKey();

    Data& data() { return mData; }
    const Data& data() const { return mData; }

    void setOpacity(float aOpacity) { mData.setOpacity(aOpacity); }
    const float& opacity() const { return mData.opacity(); }

    virtual TimeKeyType type() const { return TimeKeyType_Opa; }
    virtual TimeKey* createClone();
    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

private:
    Data mData;
};

} // namespace core

#endif // CORE_OPAKEY_H
