#ifndef CTRL_TIMELINEFOCUS_H
#define CTRL_TIMELINEFOCUS_H

#include <QVector>
#include "util/PlacePointer.h"
#include "util/LifeLink.h"
#include "core/TimeKeyPos.h"
#include "core/TimeLineEvent.h"
#include "ctrl/TimeLineRow.h"
#include "ctrl/TimeLineScale.h"

namespace ctrl
{

class TimeLineFocus
{
public:
    struct SingleFocus
    {
        SingleFocus();
        bool isValid() const;
        core::ObjectNode* node;
        core::TimeKeyPos pos;
    };

    TimeLineFocus(
            const QVector<TimeLineRow>& aRows,
            const TimeLineScale& aScale,
            int aMargin);

    SingleFocus reset(const QPoint& aPoint);
    bool update(const QPoint& aPoint);
    bool select(core::TimeLineEvent& aEvent);
    bool isInRange(const QPoint& aPoint) const;
    bool hasRange() const;
    void clear();
    bool viewIsChanged() const;
    QRect visualRect() const;
    void moveBoundingRect(int aAddFrame);

private:
    SingleFocus updateImpl(bool aForceSingle);
    QRect boundingRect() const;

    const QVector<TimeLineRow>& mRows;
    const TimeLineScale& mScale;
    util::PlacePointer<util::LifeLink> mFocusLink;
    QPoint mPoint;
    QRect mRange;
    bool mFoundFocus;
    bool mViewIsChanged;
    int mMargin;
    int mRadius;
};

} // namespace ctrl

#endif // CTRL_TIMELINEFOCUS_H
