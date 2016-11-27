#include "core/MoveKey.h"
#include "core/Constant.h"

namespace core
{
const MoveKey::SplineType MoveKey::kDefaultSplineType = MoveKey::SplineType_CatmullRom;

//-------------------------------------------------------------------------------------------------
MoveKey::Data::Data()
    : easing()
    , spline(kDefaultSplineType)
    , pos()
{
}

void MoveKey::Data::clamp()
{
    pos.setX(xc_clamp(pos.x(), Constant::transMin(), Constant::transMax()));
    pos.setY(xc_clamp(pos.y(), Constant::transMin(), Constant::transMax()));
}

//-------------------------------------------------------------------------------------------------
MoveKey::MoveKey()
    : mData()
{
}

bool MoveKey::serialize(Serializer& aOut) const
{
    aOut.write(mData.easing);
    aOut.write((int)mData.spline);
    aOut.write(mData.pos);
    return aOut.checkStream();
}

bool MoveKey::deserialize(Deserializer &aIn)
{
    aIn.pushLogScope("MoveKey");

    // easing
    if (!aIn.read(mData.easing))
    {
        return aIn.errored("invalid easing param");
    }

    // spline type
    {
        int splineIdx = 0;
        aIn.read(splineIdx);
        if (splineIdx < 0 || SplineType_TERM <= splineIdx)
        {
            return aIn.errored("invalid spline type");
        }
        mData.spline = (SplineType)splineIdx;
    }

    // position
    aIn.read(mData.pos);
    mData.clamp();

    aIn.popLogScope();
    return aIn.checkStream();
}

} // namespace core
