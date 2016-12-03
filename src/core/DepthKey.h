#ifndef CORE_DEPTHKEY_H
#define CORE_DEPTHKEY_H

#include "util/Easing.h"
#include "core/TimeKey.h"

namespace core
{

class DepthKey : public TimeKey
{
public:
    class Data
    {
        util::Easing::Param mEasing;
        float mDepth;
        void clamp();
    public:
        Data();
        util::Easing::Param& easing() { return mEasing; }
        const util::Easing::Param& easing() const { return mEasing; }
        void setDepth(float aDepth) { mDepth = aDepth; clamp(); }
        void addDepth(float aAdd) { mDepth += aAdd; clamp(); }
        const float& depth() const { return mDepth; }
    };

    DepthKey();

    void setDepth(float aDepth) { mData.setDepth(aDepth); }
    const float& depth() const { return mData.depth(); }

    Data& data() { return mData; }
    const Data& data() const { return mData; }

    virtual TimeKeyType type() const { return TimeKeyType_Depth; }
    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

private:
    Data mData;
};

} // namespace core

#endif // CORE_DEPTHKEY_H
