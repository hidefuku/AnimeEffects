#ifndef CORE_ABSTRACTCURSOR_H
#define CORE_ABSTRACTCURSOR_H

#include <QMouseEvent>
#include <QPoint>
#include "core/CameraInfo.h"

namespace core
{

class AbstractCursor
{
public:
    enum Status
    {
        Status_Press,
        Status_Move,
        Status_Release
    };

    enum Button
    {
        Button_None,
        Button_Left,
        Button_Middle,
        Button_Right
    };

    AbstractCursor();

    void setMousePress(QMouseEvent* aEvent, const CameraInfo& aCameraInfo);
    void setMouseMove(QMouseEvent* aEvent, const CameraInfo& aCameraInfo);
    void setMouseRelease(QMouseEvent* aEvent, const CameraInfo& aCameraInfo);
    void setMouseDoubleClick(QMouseEvent* aEvent, const CameraInfo& aCameraInfo);

    inline Button button() const { return mButton; }
    inline Status status() const { return mStatus; }
    inline QPoint screenPoint() const { return mScreenPoint; }
    inline QVector2D screenPos() const { return mScreenPos; }
    inline QVector2D screenVel() const { return mScreenVel; }
    inline QPoint worldPoint() const { return mWorldPos.toPoint(); }
    inline QVector2D worldPos() const { return mWorldPos; }
    inline QVector2D worldVel() const { return mWorldVel; }
    inline bool isDouble() const { return mIsDouble; }

    inline bool isLeftPressState() const { return mStatus == Status_Press && mButton == Button_Left; }
    inline bool isLeftMoveState() const { return mStatus == Status_Move && mButton == Button_Left; }
    inline bool isLeftReleaseState() const { return mStatus == Status_Release && mButton == Button_Left; }
    inline bool isRightPressState() const { return mStatus == Status_Press && mButton == Button_Right; }
    inline bool isRightMoveState() const { return mStatus == Status_Move && mButton == Button_Right; }
    inline bool isRightReleaseState() const { return mStatus == Status_Release && mButton == Button_Right; }
    inline bool isPressState() const { return mStatus == Status_Press; }

private:
    Status mStatus;
    Button mButton;
    QPoint mScreenPoint;
    QVector2D mScreenPos;
    QVector2D mScreenVel;
    QVector2D mWorldPos;
    QVector2D mWorldVel;
    bool mIsPressing;
    bool mIsDouble;
};

} // namespace core

#endif // CORE_ABSTRACTCURSOR_H
