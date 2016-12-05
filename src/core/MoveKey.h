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

    class Data
    {
        util::Easing::Param mEasing;
        SplineType mSpline;
        QVector2D mPos;
        void clamp();
    public:
        Data();

        util::Easing::Param& easing() { return mEasing; }
        const util::Easing::Param& easing() const { return mEasing; }

        void setSpline(SplineType aType) { mSpline = aType; }
        SplineType spline() const { return mSpline; }

        void setPos(const QVector2D& aPos) { mPos = aPos; clamp(); }
        void addPos(const QVector2D& aAdd) { mPos += aAdd; clamp(); }
        const QVector2D& pos() const { return mPos; }
    };

    static std::array<QVector2D, 2> getCatmullRomVels(
            const MoveKey* aKey0, const MoveKey* aKey1,
            const MoveKey* aKey2, const MoveKey* aKey3);

    MoveKey();

    void setPos(const QVector2D& aPos) { mData.setPos(aPos); }
    const QVector2D& pos() const { return mData.pos(); }

    Data& data() { return mData; }
    const Data& data() const { return mData; }

    virtual TimeKeyType type() const { return TimeKeyType_Move; }
    virtual TimeKey* createClone();
    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

private:
    Data mData;
};

} // namespace core

#endif // CORE_MOVEKEY_H
