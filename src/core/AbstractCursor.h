#ifndef CORE_ABSTRACTCURSOR_H
#define CORE_ABSTRACTCURSOR_H

#include <functional>
#include <QMouseEvent>
#include <QPoint>
#include "core/CameraInfo.h"

namespace core
{

class AbstractCursor
{
public:
    enum Event
    {
        Event_Press,
        Event_Move,
        Event_Release,
        Event_TERM
    };

    enum Button
    {
        Button_Left,
        Button_Middle,
        Button_Right,
        Button_TERM
    };

    AbstractCursor();

    /// update cursor by mouse events.
    /// these functions will return true when the caller should reflect the new cursor event.
    /// @{
    bool setMousePress(QMouseEvent* aEvent, const CameraInfo& aCameraInfo);
    bool setMouseMove(QMouseEvent* aEvent, const CameraInfo& aCameraInfo);
    bool setMouseRelease(QMouseEvent* aEvent, const CameraInfo& aCameraInfo);
    bool setMouseDoubleClick(QMouseEvent* aEvent, const CameraInfo& aCameraInfo);
    /// @}
    void suspendEvent(const std::function<void()>& aEventReflector);
    void resumeEvent();

    inline Button eventButton() const { return mEventButton; }
    inline Event eventType() const { return mEventType; }
    inline QPoint screenPoint() const { return mScreenPoint; }
    inline QVector2D screenPos() const { return mScreenPos; }
    inline QVector2D screenVel() const { return mScreenVel; }
    inline QPoint worldPoint() const { return mWorldPos.toPoint(); }
    inline QVector2D worldPos() const { return mWorldPos; }
    inline QVector2D worldVel() const { return mWorldVel; }

    inline bool isPressing(Button aButton) const { return mIsPressing.at(aButton); }
    inline bool isLeftPressing() const { return mIsPressing[Button_Left]; }
    inline bool isMiddlePressing() const { return mIsPressing[Button_Middle]; }
    inline bool isRightPressing() const { return mIsPressing[Button_Right]; }

    bool isLeftPressState() const;
    bool isLeftMoveState() const;
    bool isLeftReleaseState() const;
    bool isRightPressState() const;
    bool isRightMoveState() const;
    bool isRightReleaseState() const;
    bool isPressState() const;

private:
    Event mEventType;
    Button mEventButton;
    std::array<bool, Button_TERM> mIsPressing;
    std::array<bool, Button_TERM> mIsDouble;
    QPoint mScreenPoint;
    QVector2D mScreenPos;
    QVector2D mScreenVel;
    QVector2D mWorldPos;
    QVector2D mWorldVel;

    int mSuspendedCount;
    std::array<bool, Button_TERM> mBlankAfterSuspending;
};

} // namespace core

#endif // CORE_ABSTRACTCURSOR_H
