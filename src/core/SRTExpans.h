#ifndef CORE_SRTEXPANS
#define CORE_SRTEXPANS

#include <QRect>
#include <QMatrix4x4>
#include "util/Range.h"
#include "util/FergusonCoonsSpline.h"
#include "core/Frame.h"

namespace core
{

class SRTExpans
{
public:
    typedef util::FergusonCoonsSpline<QVector3D> SplineType;

    static QMatrix4x4 getLocalSRMatrix(float aRotate, const QVector2D& aScale);

    SRTExpans();

    void setSplineCache(const util::Range& aFrame) { mSplineCache = aFrame; }
    void clearSplineCache() { mSplineCache = util::Range(-1, 0); }
    bool hasSplineCache(Frame aFrame) const;

    void setPos(const QVector2D& aPos) { mPos = aPos; }
    QVector2D pos() const { return mPos; }

    void setRotate(float aRotate) { mRotate = aRotate; }
    float rotate() const { return mRotate; }

    void setScale(const QVector2D& aScale) { mScale = aScale; }
    QVector2D scale() const { return mScale; }

    void setCentroid(const QVector2D& aValue) { mCentroid = aValue; }
    QVector2D centroid() const { return mCentroid; }

    SplineType& spline() { return mSpline; }
    const SplineType& spline() const { return mSpline; }

    QMatrix4x4 localCSRTMatrix() const;
    QMatrix4x4 localSRTMatrix() const;
    QMatrix4x4 localSRMatrix() const;

    void setParentMatrix(const QMatrix4x4& aWorldCSRTMtx);
    const QMatrix4x4& parentMatrix() const;

    QMatrix4x4 worldCSRTMatrix() const;
    QMatrix4x4 worldSRTMatrix() const;

private:
    QVector2D mPos;
    float mRotate;
    QVector2D mScale;
    QVector2D mCentroid;
    SplineType mSpline;
    QMatrix4x4 mParentMatrix;
    util::Range mSplineCache;
};

} // namespace core

#endif // CORE_SRTEXPANS

