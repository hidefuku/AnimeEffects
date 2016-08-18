#ifndef CORE_OPAKEY_H
#define CORE_OPAKEY_H

#include "util/Easing.h"
#include "core/TimeKey.h"

namespace core
{

class OpaKey : public TimeKey
{
public:
    struct Data
    {
        Data();
        bool isZero() const;
        void clamp();
        util::Easing::Param easing;
        float opacity;
    };

    OpaKey();

    Data& data() { return mData; }
    const Data& data() const { return mData; }

    virtual TimeKeyType type() const { return TimeKeyType_Opa; }
    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

private:
    Data mData;
};

} // namespace core

#endif // CORE_OPAKEY_H
