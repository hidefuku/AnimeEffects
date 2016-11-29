#ifndef CORE_SCALEKEY_H
#define CORE_SCALEKEY_H

#include "util/Easing.h"
#include "core/TimeKey.h"

namespace core
{

class ScaleKey : public TimeKey
{
public:
    class Data
    {
        util::Easing::Param mEasing;
        QVector2D mScale;
        void clamp();

    public:
        Data();

        util::Easing::Param& easing() { return mEasing; }
        const util::Easing::Param& easing() const { return mEasing; }

        void setScale(const QVector2D& aScale) { mScale = aScale; clamp(); }
        void setScaleX(float aScaleX) { mScale.setX(aScaleX); clamp(); }
        void setScaleY(float aScaleY) { mScale.setY(aScaleY); clamp(); }
        const QVector2D& scale() const { return mScale; }
    };

    ScaleKey();

    void setScale(const QVector2D& aScale) { mData.setScale(aScale); }
    const QVector2D& scale() const { return mData.scale(); }

    Data& data() { return mData; }
    const Data& data() const { return mData; }

    virtual TimeKeyType type() const { return TimeKeyType_Scale; }
    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

private:
    Data mData;
};

} // namespace core

#endif // CORE_SCALEKEY_H
