#include "util/MathUtil.h"
#include "core/Constant.h"
#include "ctrl/bone/bone_GeoBuilder.h"

using namespace core;

namespace ctrl {
namespace bone {

void GeoBuilder::build(QList<core::Bone2*>& aTopBones)
{
    GeoBuilder builder(aTopBones);
    (void)builder;
}

GeoBuilder::GeoBuilder(QList<core::Bone2*>& aTopBones)
    : mOutlines()
{
    // create a united outline of bones
    for (auto topBone : aTopBones)
    {
        Bone2::Iterator itr(topBone);
        while (itr.hasNext())
        {
            Bone2* bone = itr.next();
            XC_PTR_ASSERT(bone);

            QPolygonF boneGeo = getGeo(*bone);
            if (boneGeo.empty())
            {
                continue;
            }

            if (mOutlines.empty())
            {
                mOutlines = boneGeo;
                continue;
            }

            mOutlines = mOutlines.united(boneGeo);
        }
    }

    if (mOutlines.empty())
    {
        return;
    }

    // create bone influence shape
    for (auto topBone : aTopBones)
    {
        Bone2::Iterator itr(topBone);
        while (itr.hasNext())
        {
            Bone2* bone = itr.next();
            XC_PTR_ASSERT(bone);
            Bone2* parent = bone->parent();
            if (!parent) continue;

            const QVector2D ppos = parent->worldPos();
            const util::Segment2D seg(ppos, bone->worldPos() - ppos);

            const QVector2D parentDir = parent->parent() ?
                                            ppos - parent->parent()->worldPos() :
                                            QVector2D();

            // extended range
            static const float kRangeExtendRate = 0.3f;
            const float rangeExtend = kRangeExtendRate *
                    std::max(bone->range(0).x(), bone->range(1).x());
            auto range0 = bone->range(0) + QVector2D(rangeExtend, rangeExtend);
            auto range1 = bone->range(1) + QVector2D(rangeExtend, rangeExtend);

            QPolygonF boneGeo = getGeo(seg, range0, range1);
            if (boneGeo.empty())
            {
                continue;
            }

            auto infl = boneGeo.intersected(mOutlines);
            // erase a appended close vertex at last index of QPolygonF
            eraseUselessCloser(infl);

            // erase divided influence polygons
            // which doesn't contains a bone segment pos.
            eraseDividedPolygons(infl, seg.start);

            if (!infl.empty())
            {
                mInfluenceList.push_back(infl);

                // set the polygon to the bone
                BoneShape shape;
                shape.setSegment(seg);
                shape.setRadius(range0, range1);
                shape.setPolygon(infl);
                shape.setRootBendFromDirections(seg.dir, parentDir);
                for (auto child : bone->children())
                {
                    shape.adjustTailBendFromDirections(
                                seg.dir, child->worldPos() - seg.end());
                }

                bone->setShape(shape);
            }
        }
    }
}

void GeoBuilder::eraseDividedPolygons(QPolygonF& aTarget, const QVector2D& aRoot)
{
    if (aTarget.count() > 3)
    {
        const QPointF root = aRoot.toPointF();
        PolygonList comprimes;
        separateToComprimes(aTarget, comprimes);
        for (auto& comprime : comprimes)
        {
            if (comprime.containsPoint(root, Qt::OddEvenFill))
            {
                aTarget = comprime;
                return;
            }
        }
    }
    aTarget = QPolygonF();
}

void GeoBuilder::eraseUselessCloser(QPolygonF& aTarget)
{
    const int count = aTarget.count();
    if (count > 3)
    {
        if (aTarget[0] == aTarget[count - 1])
        {
            const int close = aTarget.indexOf(aTarget[count - 2]);
            if (close >= 0 && close != count - 2)
            {
                aTarget.resize(count - 1);
            }
        }
    }
}

void GeoBuilder::separateToComprimes(const QPolygonF& aSrc, PolygonList& aDst)
{
    aDst.clear();

    // separate to coprime polygons
    const int count = aSrc.count();

    int i = 0;
    while (i < count - 1)
    {
        const QPointF bgn = aSrc[i];
        bool found = false;
        for (int k = i + 1; k < count; ++k)
        {
            if (bgn == aSrc[k])
            {
                const int num = k - i + 1;
                if (num > 3)
                {
                    aDst.push_back(QPolygonF(aSrc.mid(i, num)));
                }
                i = k + 1;
                found = true;
                break;
            }
        }

        if (!found) ++i;
    }
}

QPolygonF GeoBuilder::getGeo(const core::Bone2& aBone)
{
    if (!aBone.parent()) return QPolygonF();
    if (!aBone.hasValidRange()) return QPolygonF();

    static const int kFanDivide = 16;
    const QVector2D worldPos[2] = { aBone.parent()->worldPos(), aBone.worldPos() };
    const QVector2D dir(worldPos[1] - worldPos[0]);
    const float len = dir.length();

    if (len < core::Constant::normalizable()) QPolygonF();

    QPolygonF result;
    const QVector2D v = dir.normalized();
    const QVector2D h = util::MathUtil::getRotateVector90Deg(v);

    for (int t = 0; t < 2; ++t)
    {
        const QVector2D center = worldPos[t];
        const float wing = aBone.range(t).x();
        const float vrange = aBone.range(t).y();
        const float vscale = xc_divide(vrange, wing, Constant::dividable(), 1.0f);
        const QVector2D swing = (t == 0) ? (wing * h) : (-wing * h);

        for (int i = 0; i <= kFanDivide; ++i)
        {
            const float rotate = M_PI * i / (float)kFanDivide;
            const QVector2D offs = util::MathUtil::getRotateVectorRad(swing, rotate);
            const float hdot = QVector2D::dotProduct(h, offs);
            const float vdot = QVector2D::dotProduct(v, offs);
            const QPointF pos = (center + hdot * h + vscale * vdot * v).toPointF();
            if (result.empty() || result.back() != pos)
            {
                result.append(pos);
            }
        }
    }

    if (result.empty() || result.count() < 3)
    {
        return QPolygonF();
    }

    // close polygon
    if (result.back() != result.first())
    {
        result.append(result.first());
    }

    return result;
}

QPolygonF GeoBuilder::getGeo(
        const util::Segment2D& aSeg,
        const QVector2D& aRange0,
        const QVector2D& aRange1)
{
    if (aRange0.x() <= 0.0f && aRange1.x() <= 0.0f) return QPolygonF();

    static const int kFanDivide = 16;
    const QVector2D worldPos[2] = { aSeg.start, aSeg.end() };
    const QVector2D dir(aSeg.dir);
    const float len = dir.length();

    if (len < core::Constant::normalizable()) QPolygonF();

    QPolygonF result;
    const QVector2D v = dir.normalized();
    const QVector2D h = util::MathUtil::getRotateVector90Deg(v);

    for (int t = 0; t < 2; ++t)
    {
        const QVector2D center = worldPos[t];
        const float wing = (t == 0) ? aRange0.x() : aRange1.x();
        const float vrange = (t == 0) ? aRange0.y() : aRange1.y();
        const float vscale = xc_divide(vrange, wing, Constant::dividable(), 1.0f);
        const QVector2D swing = (t == 0) ? (wing * h) : (-wing * h);

        for (int i = 0; i <= kFanDivide; ++i)
        {
            const float rotate = M_PI * i / (float)kFanDivide;
            const QVector2D offs = util::MathUtil::getRotateVectorRad(swing, rotate);
            const float hdot = QVector2D::dotProduct(h, offs);
            const float vdot = QVector2D::dotProduct(v, offs);
            const QPointF pos = (center + hdot * h + vscale * vdot * v).toPointF();
            if (result.empty() || result.back() != pos)
            {
                result.append(pos);
            }
        }
    }

    if (result.empty() || result.count() < 3)
    {
        return QPolygonF();
    }

    // close polygon
    if (result.back() != result.first())
    {
        result.append(result.first());
    }

    return result;
}

} // namespace bone
} // namespace ctrl
