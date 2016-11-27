#include "core/ScaleKey.h"
#include "core/Constant.h"

namespace core
{
//-------------------------------------------------------------------------------------------------
ScaleKey::Data::Data()
    : easing()
    , scale(1.0f, 1.0f)
{
}

void ScaleKey::Data::clamp()
{
    scale.setX(xc_clamp(scale.x(), Constant::scaleMin(), Constant::scaleMax()));
    scale.setY(xc_clamp(scale.y(), Constant::scaleMin(), Constant::scaleMax()));
}

//-------------------------------------------------------------------------------------------------
ScaleKey::ScaleKey()
    : mData()
{
}

bool ScaleKey::serialize(Serializer& aOut) const
{
    aOut.write(mData.easing);
    aOut.write(mData.scale);
    return aOut.checkStream();
}

bool ScaleKey::deserialize(Deserializer &aIn)
{
    aIn.pushLogScope("ScaleKey");

    // easing
    if (!aIn.read(mData.easing))
    {
        return aIn.errored("invalid easing param");
    }

    aIn.read(mData.scale);
    mData.clamp();

    aIn.popLogScope();
    return aIn.checkStream();
}

} // namespace core
