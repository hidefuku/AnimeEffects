#ifndef CTRL_TIME_FOCUSER_H
#define CTRL_TIME_FOCUSER_H

#include <QVector>
#include "util/PlacePointer.h"
#include "util/LifeLink.h"
#include "core/TimeKeyPos.h"
#include "core/TimeLineEvent.h"
#include "ctrl/TimeLineRow.h"
#include "ctrl/time/time_Scaler.h"

namespace ctrl {
namespace time {

class Focuser
{
public:
    struct SingleFocus
    {
        SingleFocus();
        bool isValid() const;
        core::ObjectNode* node;
        core::TimeKeyPos pos;
    };

    Focuser(
            const QVector<TimeLineRow>& aRows,
            const Scaler& aScale,
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
    const Scaler& mScale;
    util::PlacePointer<util::LifeLink> mFocusLink;
    QPoint mPoint;
    QRect mRange;
    bool mFoundFocus;
    bool mViewIsChanged;
    int mMargin;
    int mRadius;
};

} // namespace time
} // namespace ctrl

#endif // CTRL_TIME_FOCUSER_H
