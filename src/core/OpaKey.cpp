#include "core/OpaKey.h"

namespace core
{

OpaKey::Data::Data()
    : easing()
    , opacity(1.0f)
{
}

bool OpaKey::Data::isZero() const
{
    return opacity == 0.0f;
}

void OpaKey::Data::clamp()
{
    opacity = xc_clamp(opacity, 0.0f, 1.0f);
}

OpaKey::OpaKey()
    : mData()
{
}

bool OpaKey::serialize(Serializer& aOut) const
{
    aOut.write(mData.easing);
    aOut.write(mData.opacity);
    return aOut.checkStream();
}

bool OpaKey::deserialize(Deserializer& aIn)
{
    aIn.pushLogScope("OpaKey");

    if (!aIn.read(mData.easing))
    {
        return aIn.errored("invalid easing param");
    }

    aIn.read(mData.opacity);
    mData.clamp();

    aIn.popLogScope();
    return aIn.checkStream();
}

} // namespace core

