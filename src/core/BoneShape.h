#ifndef CORE_BONESHAPE_H
#define CORE_BONESHAPE_H

#include <array>
#include <QPolygonF>
#include "util/Segment2D.h"
#include "core/Serializer.h"
#include "core/Deserializer.h"

namespace core
{

class BoneShape
{
public:
    class BendRange
    {
    public:
        BendRange();
        float getWeight(float aBendAngle) const;
        bool isValid() const;
        float angle[2]; // plus range and minus range
    };

    BoneShape();
    void setSegment(const util::Segment2D& aSegment);
    void setRadius(const QVector2D& aRoot, const QVector2D& aTail);
    void setPolygon(const QPolygonF& aPolygon);
    void setBendRange(const BendRange& aRoot, const BendRange& aTail);
    void setRootBendFromDirections(const QVector2D& aMyDir,
                                   const QVector2D& aParentDir);
    void adjustTailBendFromDirections(const QVector2D& aMyDir,
                                      const QVector2D& aChildDir);

    float influence(const QVector2D& aPos) const;

    // serialize
    bool serialize(Serializer& aOut) const;
    bool deserialize(Deserializer& aIn);

private:
    void updateValidity();
    float getBoneWeight(const QVector2D& aPoint) const;
    float getBoneEllipseWeight(
            const QVector2D& aCenter, const QVector2D& aVUnit,
            const QVector2D& aRadius, const QVector2D& aPoint) const;
    float getWeakness(float aRate) const;

    bool mIsValid;
    util::Segment2D mSegment;
    QVector2D mVUnit;
    float mVDirAngle;
    float mLength;
    std::array<QVector2D, 2> mRadius;
    QRectF mBounding;
    QPolygonF mPolygon;
    BendRange mRootBendRange;
    BendRange mTailBendRange;
};

} // namespace core

#endif // CORE_BONESHAPE_H
