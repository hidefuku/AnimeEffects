#include "core/DepthKey.h"
#include "core/Constant.h"

namespace core
{
//-------------------------------------------------------------------------------------------------
DepthKey::Data::Data()
    : mEasing()
    , mDepth(0.0f)
{
}

void DepthKey::Data::clamp()
{
    mDepth = xc_clamp(mDepth, Constant::transMin(), Constant::transMax());
}

//-------------------------------------------------------------------------------------------------
DepthKey::DepthKey()
    : mData()
{
}

TimeKey* DepthKey::createClone()
{
    auto newKey = new DepthKey();
    newKey->mData = this->mData;
    return newKey;
}

bool DepthKey::serialize(Serializer& aOut) const
{
    aOut.write(mData.easing());
    aOut.write(mData.depth());
    return aOut.checkStream();
}

bool DepthKey::deserialize(Deserializer &aIn)
{
    aIn.pushLogScope("DepthKey");

    // easing
    if (!aIn.read(mData.easing()))
    {
        return aIn.errored("invalid easing param");
    }

    float depth = 0.0f;
    aIn.read(depth);
    mData.setDepth(depth);

    aIn.popLogScope();
    return aIn.checkStream();
}

} // namespace core
