#ifndef CORE_SRTEXPANS
#define CORE_SRTEXPANS

#include <QRect>
#include <QMatrix4x4>
#include "util/Range.h"
#include "util/FergusonCoonsSpline.h"
#include "core/SRTKey.h"
#include "core/Frame.h"

namespace core
{

class SRTExpans
{
public:
    typedef util::FergusonCoonsSpline<QVector3D> SplineType;

    SRTExpans()
        : mData()
        , mSpline()
        , mParentMatrix()
    {}

    void setSplineCache(const util::Range& aFrame) { mSplineCache = aFrame; }
    void clearSplineCache() { mSplineCache = util::Range(-1, 0); }
    bool hasSplineCache(Frame aFrame) const
    {
        if (aFrame <= 0 || mSplineCache.min() <= 0) return false;
        if (mSplineCache.isNegative())
            return aFrame >= mSplineCache.min() || aFrame <= mSplineCache.max();
        else
            return mSplineCache.contains(aFrame.getDecimal());
    }

    SRTKey::Data& data() { return mData; }
    const SRTKey::Data& data() const { return mData; }

    SplineType& spline() { return mSpline; }
    const SplineType& spline() const { return mSpline; }

    QMatrix4x4& parentMatrix() { return mParentMatrix; }
    const QMatrix4x4& parentMatrix() const { return mParentMatrix; }

    QMatrix4x4 worldMatrix() const { return mParentMatrix * mData.localMatrix(); }

private:
    SRTKey::Data mData;
    SplineType mSpline;
    QMatrix4x4 mParentMatrix;
    util::Range mSplineCache;
};

} // namespace core

#endif // CORE_SRTEXPANS

