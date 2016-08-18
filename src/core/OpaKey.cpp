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
    aOut.write((int)mData.easing.type);
    aOut.write((int)mData.easing.range);
    aOut.write(mData.easing.weight);
    aOut.write(mData.opacity);
    return aOut.checkStream();
}

bool OpaKey::deserialize(Deserializer& aIn)
{
    aIn.pushLogScope("OpaKey");

    aIn.read((int&)mData.easing.type);
    aIn.read((int&)mData.easing.range);
    aIn.read(mData.easing.weight);
    if (!mData.easing.isValidParam())
    {
        return aIn.errored("invalid easing param");
    }

    aIn.read(mData.opacity);
    mData.clamp();

    aIn.popLogScope();
    return aIn.checkStream();
}

} // namespace core

