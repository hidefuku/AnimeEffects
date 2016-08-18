#ifndef CTRL_BONE_GEOBUILDER_H
#define CTRL_BONE_GEOBUILDER_H

#include <QPolygonF>
#include "util/Segment2D.h"
#include "core/Bone2.h"

namespace ctrl {
namespace bone {

class GeoBuilder
{
public:
    typedef QList<QPolygonF> PolygonList;

    static void build(QList<core::Bone2*>& aTopBones);
    GeoBuilder(QList<core::Bone2*>& aTopBones);
    const QPolygonF& outlines() const { return mOutlines; }
    const PolygonList& influence() const { return mInfluenceList; }

private:
    static void eraseUselessCloser(QPolygonF& aTarget);
    static void eraseDividedPolygons(QPolygonF& aTarget, const QVector2D& aRoot);
    static void separateToComprimes(const QPolygonF& aSrc, PolygonList& aDst);
    static QPolygonF getGeo(const core::Bone2& aBone);
    static QPolygonF getGeo(
            const util::Segment2D& aSeg,
            const QVector2D& aRange0,
            const QVector2D& aRange1);

    QPolygonF mOutlines;
    PolygonList mInfluenceList;
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_GEOBUILDER_H
