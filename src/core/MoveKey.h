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
        QVector2D mCentroid;
        void clampPos();
        void clampCentroid();
    public:
        Data();

        util::Easing::Param& easing() { return mEasing; }
        const util::Easing::Param& easing() const { return mEasing; }

        void setSpline(SplineType aType) { mSpline = aType; }
        SplineType spline() const { return mSpline; }

        void setPos(const QVector2D& aPos) { mPos = aPos; clampPos(); }
        void addPos(const QVector2D& aAdd) { mPos += aAdd; clampPos(); }
        const QVector2D& pos() const { return mPos; }

        void setCentroid(const QVector2D& aValue) { mCentroid = aValue; clampCentroid(); }
        void addCentroid(const QVector2D& aAdd) { mCentroid += aAdd; clampCentroid(); }
        const QVector2D& centroid() const { return mCentroid; }
    };

    static std::array<QVector2D, 2> getCatmullRomVels(
            const MoveKey* aKey0, const MoveKey* aKey1,
            const MoveKey* aKey2, const MoveKey* aKey3);

    MoveKey();

    void setPos(const QVector2D& aPos) { mData.setPos(aPos); }
    void addPos(const QVector2D& aAdd) { mData.addPos(aAdd);}
    const QVector2D& pos() const { return mData.pos(); }

    void setCentroid(const QVector2D& aValue) { mData.setCentroid(aValue); }
    void addCentroid(const QVector2D& aAdd) { mData.addCentroid(aAdd); }
    const QVector2D& centroid() const { return mData.centroid(); }

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
