#include "core/ScaleKey.h"
#include "core/Constant.h"

namespace core
{
//-------------------------------------------------------------------------------------------------
ScaleKey::Data::Data()
    : mEasing()
    , mScale(1.0f, 1.0f)
{
}

void ScaleKey::Data::clamp()
{
    mScale.setX(xc_clamp(mScale.x(), Constant::scaleMin(), Constant::scaleMax()));
    mScale.setY(xc_clamp(mScale.y(), Constant::scaleMin(), Constant::scaleMax()));
}

//-------------------------------------------------------------------------------------------------
ScaleKey::ScaleKey()
    : mData()
{
}

TimeKey* ScaleKey::createClone()
{
    auto newKey = new ScaleKey();
    newKey->mData = this->mData;
    return newKey;
}

bool ScaleKey::serialize(Serializer& aOut) const
{
    aOut.write(mData.easing());
    aOut.write(mData.scale());
    return aOut.checkStream();
}

bool ScaleKey::deserialize(Deserializer &aIn)
{
    aIn.pushLogScope("ScaleKey");

    // easing
    if (!aIn.read(mData.easing()))
    {
        return aIn.errored("invalid easing param");
    }

    QVector2D scale;
    aIn.read(scale);
    mData.setScale(scale);

    aIn.popLogScope();
    return aIn.checkStream();
}

} // namespace core
