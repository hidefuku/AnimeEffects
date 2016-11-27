#include "core/RotateKey.h"
#include "core/Constant.h"

namespace core
{
//-------------------------------------------------------------------------------------------------
RotateKey::Data::Data()
    : easing()
    , rotate(0.0f)
{
}

void RotateKey::Data::clamp()
{
    rotate = xc_clamp(rotate, Constant::rotateMin(), Constant::rotateMax());
}

//-------------------------------------------------------------------------------------------------
RotateKey::RotateKey()
    : mData()
{
}

bool RotateKey::serialize(Serializer& aOut) const
{
    aOut.write(mData.easing);
    aOut.write(mData.rotate);
    return aOut.checkStream();
}

bool RotateKey::deserialize(Deserializer &aIn)
{
    aIn.pushLogScope("RotateKey");

    // easing
    if (!aIn.read(mData.easing))
    {
        return aIn.errored("invalid easing param");
    }

    aIn.read(mData.rotate);
    mData.clamp();

    aIn.popLogScope();
    return aIn.checkStream();
}

} // namespace core
