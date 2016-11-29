#include "core/RotateKey.h"
#include "core/Constant.h"

namespace core
{
//-------------------------------------------------------------------------------------------------
RotateKey::Data::Data()
    : mEasing()
    , mRotate(0.0f)
{
}

void RotateKey::Data::clamp()
{
    mRotate = xc_clamp(mRotate, Constant::rotateMin(), Constant::rotateMax());
}

//-------------------------------------------------------------------------------------------------
RotateKey::RotateKey()
    : mData()
{
}

bool RotateKey::serialize(Serializer& aOut) const
{
    aOut.write(mData.easing());
    aOut.write(mData.rotate());
    return aOut.checkStream();
}

bool RotateKey::deserialize(Deserializer &aIn)
{
    aIn.pushLogScope("RotateKey");

    // easing
    if (!aIn.read(mData.easing()))
    {
        return aIn.errored("invalid easing param");
    }

    float rotate = 0.0f;
    aIn.read(rotate);
    mData.setRotate(rotate);

    aIn.popLogScope();
    return aIn.checkStream();
}

} // namespace core
