#include <QtMath>
#include "util/MathUtil.h"
#include "util/CollDetect.h"
#include "core/Constant.h"
#include "core/BoneShape.h"

//-------------------------------------------------------------------------------------------------
namespace
{
static const float kRangeMin = 0.1f;
} // namespace

namespace core
{

//-------------------------------------------------------------------------------------------------
BoneShape::BendRange::BendRange()
{
    angle[0] = -1.0f;
    angle[1] = 1.0f;
}

bool BoneShape::BendRange::isValid() const
{
    return angle[0] >= 0.0f && angle[1] <= 0.0f;
}

float BoneShape::BendRange::getWeight(float aBendAngle) const
{
    if (!isValid()) return 1.0f;

    // positive angle
    float posiWeight = 0.0f;
    if (angle[0] > 0.0f)
    {
        const float posiBend = (aBendAngle >= 0.0f) ?
                                   aBendAngle : (aBendAngle + 2 * M_PI);
        posiWeight = 1.0f - posiBend / angle[0];
    }
    // negative angle
    float negaWeight = 0.0f;
    if (angle[1] < 0.0f)
    {
        const float negaBend = (aBendAngle <= 0.0f) ?
                                   aBendAngle : (aBendAngle - 2 * M_PI);
        negaWeight = 1.0f - negaBend / angle[1];
    }

    return std::max(0.001f, std::max(posiWeight, negaWeight));
}

//-------------------------------------------------------------------------------------------------
BoneShape::BoneShape()
    : mIsValid()
    , mSegment()
    , mVUnit()
    , mVDirAngle()
    , mLength()
    , mRadius()
    , mBounding()
    , mPolygon()
    , mRootBendRange()
    , mTailBendRange()
{
}

void BoneShape::updateValidity()
{
    const bool hasValidRange = !mRadius[0].isNull() || !mRadius[1].isNull();
    const bool hasLength = mSegment.dir.length() >= Constant::normalizable();
    mIsValid = hasValidRange && hasLength;
    if (mIsValid)
    {
        mVUnit = mSegment.dir.normalized();
        mLength = mSegment.dir.length();
        mVDirAngle = util::MathUtil::getAngleRad(mVUnit);
    }
}

void BoneShape::setSegment(const util::Segment2D& aSegment)
{
    mSegment = aSegment;
    updateValidity();
}

void BoneShape::setRadius(const QVector2D& aRoot, const QVector2D& aTail)
{
    mRadius[0] = aRoot;
    mRadius[1] = aTail;
    updateValidity();
}

void BoneShape::setPolygon(const QPolygonF& aPolygon)
{
    mPolygon = aPolygon;
    mBounding = aPolygon.boundingRect();
}

void BoneShape::setBendRange(const BendRange& aRoot, const BendRange& aTail)
{
    mRootBendRange = aRoot;
    mTailBendRange = aTail;
}

void BoneShape::setRootBendFromDirections(const QVector2D& aMyDir,
                                          const QVector2D& aParentDir)
{
    mRootBendRange = BendRange();

    if (!aMyDir.isNull() && !aParentDir.isNull())
    {
        auto diff = util::MathUtil::getAngleDifferenceRad(aMyDir, -aParentDir);
        if (diff >= 0.0f)
        {
            mRootBendRange.angle[0] = diff;
            mRootBendRange.angle[1] = diff - 2 * M_PI;
        }
        else
        {
            mRootBendRange.angle[1] = diff;
            mRootBendRange.angle[0] = diff + 2 * M_PI;
        }
    }
}

void BoneShape::adjustTailBendFromDirections(const QVector2D& aMyDir,
                                             const QVector2D& aChildDir)
{
    if (!aMyDir.isNull() && !aChildDir.isNull())
    {
        auto diff = util::MathUtil::getAngleDifferenceRad(-aMyDir, aChildDir);

        float angle[2] = {};
        if (diff >= 0.0f)
        {
            angle[0] = diff;
            angle[1] = diff - 2 * M_PI;
        }
        else
        {
            angle[1] = diff;
            angle[0] = diff + 2 * M_PI;
        }

        if (!mTailBendRange.isValid())
        {
            mTailBendRange.angle[0] = angle[0];
            mTailBendRange.angle[1] = angle[1];
        }
        else
        {
            // reduce bend range
            mTailBendRange.angle[0] = std::min(mTailBendRange.angle[0], angle[0]);
            mTailBendRange.angle[1] = std::max(mTailBendRange.angle[1], angle[1]);
        }
    }
}

float BoneShape::influence(const QVector2D& aPos) const
{
    if (mIsValid)
    {
        const QPointF pos = aPos.toPointF();
        if (mBounding.contains(pos))
        {
            if (mPolygon.containsPoint(pos, Qt::OddEvenFill))
            {
                return getBoneWeight(aPos);
            }
        }
    }
    return 0.0f;
}

float BoneShape::getBoneEllipseWeight(
        const QVector2D& aCenter, const QVector2D& aVUnit,
        const QVector2D& aRadius, const QVector2D& aPoint) const
{
    if (aRadius.x() >= kRangeMin && aRadius.y() >= kRangeMin)
    {
#if 1
        const QVector2D dir = aPoint - aCenter;
        const QVector2D vDir = aVUnit * QVector2D::dotProduct(aVUnit, dir);
        const QVector2D hDir = dir - vDir;

        const float radius = aRadius.x();
        const float vScale = radius / aRadius.y();
        const QVector2D skewDir = hDir + vDir * vScale;
        const float skewSqlen = skewDir.lengthSquared();

        if (skewSqlen < radius * radius)
        {
            return 1.0f - qSqrt(skewSqlen) / radius;
        }
#else
        const QVector2D dir = aPoint - aCenter;
        const QVector2D vDir = aVUnit * QVector2D::dotProduct(aVUnit, dir);
        const QVector2D hDir = dir - vDir;

        const float radius = aRadius.x();
        const float vScale = radius / aRadius.y();
        const QVector2D skewDir = hDir + vDir * vScale;
        const float skewSqlen = skewDir.lengthSquared();

        if (skewSqlen < radius * radius)
        {
            const float hWeight = 1.0f - hDir.length() / aRadius.x();
            const float vWeight = 1.0f - vDir.length() / aRadius.y();
            return hWeight * vWeight;
        }

#endif
    }
    return 0.0f;
}

#if 0
float BoneShape::getBoneWeight(const QVector2D& aPoint) const
{
    const float rawSegRate = util::CollDetect::getRawSegmentRate(mSegment, aPoint);

    if (rawSegRate < 0.0f)
    { // root ellipse
        const QVector2D range = mRadius[0];
        if (range.x() < kRangeMin) return 0.0f;

        return getBoneEllipseWeight(
                    mSegment.start, mVUnit, range, aPoint) * getWeakness(0.0f);
    }
    else if (rawSegRate <= 1.0f)
    { // a point is in a bone segment range
        const QVector2D range =
                mRadius[0] * (1.0f - rawSegRate) + mRadius[1] * rawSegRate;
        if (range.x() < kRangeMin) return 0.0f;

        const QVector2D segPos = mSegment.start + rawSegRate * mSegment.dir;
        const float diffs = (aPoint - segPos).length();

        if (diffs <= range.x())
        {
            return (1.0f - diffs / range.x()) * getWeakness(rawSegRate);
        }
        else
        {
            return 0.0f;
        }
    }
    else
    { // tail ellipse
        const QVector2D range = mRadius[1];
        if (range.x() < kRangeMin) return 0.0f;

        return getBoneEllipseWeight(
                    mSegment.end(), mVUnit, range, aPoint) * getWeakness(1.0f);
    }
}
#else

float BoneShape::getBoneWeight(const QVector2D& aPoint) const
{
    //static const float kWing = (float)(M_PI * 0.25);
    //static const float kWingInv = (float)(M_PI * 0.75);
    //static const float kWing = (float)(M_PI * 0.0);
    //static const float kWingInv = (float)(M_PI * 1.0);

    float twistWeight = 1.0f;

#if 1
    // root
    const QVector2D start2point = aPoint - mSegment.start;
    if (!start2point.isNull())
    {
        const float angle = util::MathUtil::getAngleRad(start2point);
        const float diff = util::MathUtil::getAngleDifferenceRad(mVDirAngle, angle);
        //const float outress = 1.0f - std::max(std::abs(diff) - kWing, 0.0f) / kWingInv;
        //twistWeight *= outress * outress;
        const float outress = mRootBendRange.getWeight(diff);
        twistWeight *= outress * outress;
    }

    const QVector2D end2point = aPoint - mSegment.end();
    if (!end2point.isNull())
    {
        const float angle = util::MathUtil::getAngleRad(end2point);
        const float diff = util::MathUtil::getAngleDifferenceRad(mVDirAngle + M_PI, angle);
        //const float outress = 1.0f - std::max(std::abs(diff) - kWing, 0.0f) / kWingInv;
        //twistWeight *= outress * outress;
        const float outress = mTailBendRange.getWeight(diff);
        twistWeight *= outress * outress;
    }
#endif

    const float rawSegRate = util::CollDetect::getRawSegmentRate(mSegment, aPoint);
    float nearness = 1.0f;

    if (rawSegRate < 0.0f)
    { // root ellipse
        const QVector2D range = mRadius[0];
        if (range.x() >= kRangeMin)
        {
            nearness = getBoneEllipseWeight(mSegment.start, mVUnit, range, aPoint);
        }
    }
    else if (rawSegRate <= 1.0f)
    { // a point is in a bone segment range
        QVector2D range =
                mRadius[0] * (1.0f - rawSegRate) + mRadius[1] * rawSegRate;

        if (range.x() >= kRangeMin)
        {
            const QVector2D segPos = mSegment.start + rawSegRate * mSegment.dir;
            const float diffs = (aPoint - segPos).length();
            nearness = std::max(1.0f - diffs / range.x(), 0.0f);
        }
    }
    else
    { // tail ellipse
        const QVector2D range = mRadius[1];

        if (range.x() >= kRangeMin)
        {
            nearness = getBoneEllipseWeight(mSegment.end(), mVUnit, range, aPoint);
        }
    }

    //return std::min(twistWeight, nearness);
    //return twistWeight * nearness;
    const float ratio = 0.3f * nearness * nearness;
    twistWeight = ratio + (1.0f - ratio) * twistWeight;
    return twistWeight * nearness;
}
#endif


float BoneShape::getWeakness(float aRate) const
{
    const float rootV = mRadius[0].y() / mLength;
    const float tailV = mRadius[1].y() / mLength;
    float weakness = 1.0f;

    if (aRate < rootV)
    {
        weakness *= 0.5f + 0.5f * (1.0f - (rootV - aRate) / rootV);
    }

    if (aRate > 1.0f - tailV)
    {
        weakness *= 0.5f + 0.5f * (1.0f - (aRate - (1.0f - tailV)) / tailV);
    }

    return weakness;
}

bool BoneShape::serialize(Serializer& aOut) const
{
    aOut.write(mIsValid);
    aOut.write(mSegment);
    aOut.write(mVUnit);
    aOut.write(mLength);
    aOut.write(mRadius[0]);
    aOut.write(mRadius[1]);
    aOut.write(mRootBendRange.angle[0]);
    aOut.write(mRootBendRange.angle[1]);
    aOut.write(mTailBendRange.angle[0]);
    aOut.write(mTailBendRange.angle[1]);
    aOut.write(mBounding);
    aOut.write(mPolygon);

    return aOut.checkStream();
}

bool BoneShape::deserialize(Deserializer& aIn)
{
    aIn.read(mIsValid);
    aIn.read(mSegment);
    aIn.read(mVUnit);
    aIn.read(mLength);
    aIn.read(mRadius[0]);
    aIn.read(mRadius[1]);
    aIn.read(mRootBendRange.angle[0]);
    aIn.read(mRootBendRange.angle[1]);
    aIn.read(mTailBendRange.angle[0]);
    aIn.read(mTailBendRange.angle[1]);
    aIn.read(mBounding);
    aIn.read(mPolygon);

    return aIn.checkStream();

}

} // namespace core

