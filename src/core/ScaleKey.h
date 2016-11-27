#ifndef CORE_SCALEKEY_H
#define CORE_SCALEKEY_H

#include "util/Easing.h"
#include "core/TimeKey.h"

namespace core
{

class ScaleKey : public TimeKey
{
public:
    struct Data
    {
        util::Easing::Param easing;
        QVector2D scale;
        Data();
        void clamp();
    };

    ScaleKey();

    void setScale(const QVector2D& aScale) { mData.scale = aScale; }
    const QVector2D& scale() const { return mData.scale; }

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
