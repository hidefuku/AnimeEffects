#ifndef CORE_SRTKEY
#define CORE_SRTKEY

#include <QVector3D>
#include <QMatrix4x4>
#include "util/Easing.h"
#include "core/TimeKey.h"

namespace core
{

class SRTKey : public TimeKey
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
        QVector3D pos;
        float rotate;
        QVector2D scale;

        Data();
        QMatrix4x4 localMatrix() const;
        QMatrix4x4 localSRMatrix() const;
        void clamp();
        void clampPos();
        void clampRotate();
        void clampScale();
    };

    SRTKey();

    QVector3D& pos() { return mData.pos; }
    const QVector3D& pos() const { return mData.pos; }

    float& rotate() { return mData.rotate; }
    const float& rotate() const { return mData.rotate; }

    QVector2D& scale() { return mData.scale; }
    const QVector2D& scale() const { return mData.scale; }

    Data& data() { return mData; }
    const Data& data() const { return mData; }

    virtual TimeKeyType type() const { return TimeKeyType_SRT; }
    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

private:
    Data mData;
};

} // namespace core

#endif // CORE_SRTKEY

