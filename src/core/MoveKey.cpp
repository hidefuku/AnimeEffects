#include "util/MathUtil.h"
#include "core/MoveKey.h"
#include "core/Constant.h"

namespace core
{
const MoveKey::SplineType MoveKey::kDefaultSplineType = MoveKey::SplineType_CatmullRom;

//-------------------------------------------------------------------------------------------------
MoveKey::Data::Data()
    : mEasing()
    , mSpline(kDefaultSplineType)
    , mPos()
{
}

void MoveKey::Data::clamp()
{
    mPos.setX(xc_clamp(mPos.x(), Constant::transMin(), Constant::transMax()));
    mPos.setY(xc_clamp(mPos.y(), Constant::transMin(), Constant::transMax()));
}

//-------------------------------------------------------------------------------------------------
std::array<QVector2D, 2> MoveKey::getCatmullRomVels(
        const MoveKey* aKey0, const MoveKey* aKey1,
        const MoveKey* aKey2, const MoveKey* aKey3)
{
    XC_ASSERT(aKey1 && aKey2);

    std::array<QVector2D, 2> result;

    if (aKey1->data().spline() == SplineType_Linear)
    {
        aKey0 = nullptr;
    }
    if (aKey2->data().spline() == SplineType_Linear)
    {
        aKey3 = nullptr;
    }

    if (!aKey0)
    {
        const QVector2D linear = aKey2->pos() - aKey1->pos();

        if (!aKey3)
        {
            result[1] = linear;
            result[0]  = linear;
        }
        else
        {
            result[1] = 0.5f * (aKey3->pos() - aKey1->pos());
            result[0]  = util::MathUtil::getAxisInversed(linear.normalized(), result[1]);
        }
    }
    else
    {
        if (!aKey3)
        {
            const QVector2D linear = aKey2->pos() - aKey1->pos();

            result[0]  = 0.5f * (aKey2->pos() - aKey0->pos());
            result[1] = util::MathUtil::getAxisInversed(linear.normalized(), result[0]);
        }
        else
        {
            result[0]  = 0.5f * (aKey2->pos() - aKey0->pos());
            result[1] = 0.5f * (aKey3->pos() - aKey1->pos());
        }
    }
    return result;

}

//-------------------------------------------------------------------------------------------------
MoveKey::MoveKey()
    : mData()
{
}

bool MoveKey::serialize(Serializer& aOut) const
{
    aOut.write(mData.easing());
    aOut.write((int)mData.spline());
    aOut.write(mData.pos());
    return aOut.checkStream();
}

bool MoveKey::deserialize(Deserializer &aIn)
{
    aIn.pushLogScope("MoveKey");

    // easing
    if (!aIn.read(mData.easing()))
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
        mData.setSpline((SplineType)splineIdx);
    }

    // position
    QVector2D pos;
    aIn.read(pos);
    mData.setPos(pos);

    aIn.popLogScope();
    return aIn.checkStream();
}

} // namespace core
