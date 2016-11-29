#include "core/OpaKey.h"

namespace core
{

OpaKey::Data::Data()
    : mEasing()
    , mOpacity(1.0f)
{
}

bool OpaKey::Data::isZero() const
{
    return mOpacity == 0.0f;
}

void OpaKey::Data::clamp()
{
    mOpacity = xc_clamp(mOpacity, 0.0f, 1.0f);
}

OpaKey::OpaKey()
    : mData()
{
}

bool OpaKey::serialize(Serializer& aOut) const
{
    aOut.write(mData.easing());
    aOut.write(mData.opacity());
    return aOut.checkStream();
}

bool OpaKey::deserialize(Deserializer& aIn)
{
    aIn.pushLogScope("OpaKey");

    if (!aIn.read(mData.easing()))
    {
        return aIn.errored("invalid easing param");
    }

    float opa = 1.0f;
    aIn.read(opa);
    mData.setOpacity(opa);

    aIn.popLogScope();
    return aIn.checkStream();
}

} // namespace core

