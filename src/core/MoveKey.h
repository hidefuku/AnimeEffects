#ifndef CORE_MOVEKEY_H
#define CORE_MOVEKEY_H

#include <QVector2D>
#include "util/Easing.h"
#include "core/TimeKey.h"

namespace core
{

class MoveKey : public TimeKey
{
public:
    enum SplineType
    {
        SplineType_Linear,
        SplineType_CatmullRom,
        SplineType_TERM
    };
    static const SplineType kDefaultSplineType;

    struct Data
    {
        util::Easing::Param easing;
        SplineType spline;
        QVector2D pos;
        Data();
        void clamp();
    };

    MoveKey();

    void setPos(const QVector2D& aPos) { mData.pos = aPos; }
    const QVector2D& pos() const { return mData.pos; }

    Data& data() { return mData; }
    const Data& data() const { return mData; }

    virtual TimeKeyType type() const { return TimeKeyType_Move; }
    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

private:
    Data mData;
};

} // namespace core

#endif // CORE_MOVEKEY_H
