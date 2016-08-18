#include "core/AbstractCursor.h"
#include "XC.h"

namespace core
{

AbstractCursor::AbstractCursor()
    : mStatus(Status_Release)
    , mButton(Button_None)
    , mScreenPoint()
    , mScreenPos()
    , mScreenVel()
    , mWorldPos()
    , mWorldVel()
    , mIsPressing(false)
    , mIsDouble(false)
{
}


void AbstractCursor::setMousePress(QMouseEvent* aEvent, const CameraInfo& aCameraInfo)
{
    mStatus = Status_Press;
    switch (aEvent->button())
    {
    case Qt::LeftButton:  mButton = Button_Left;   break;
    case Qt::MidButton:   mButton = Button_Middle; break;
    case Qt::RightButton: mButton = Button_Right;  break;
    default:              mButton = Button_None;   break;
    }

    mScreenPoint = aEvent->pos();
    mScreenPos = QVector2D(mScreenPoint);
    mScreenVel = QVector2D(0.0f, 0.0f);
    mWorldPos = aCameraInfo.toWorldPos(mScreenPos);
    mWorldVel = QVector2D(0.0f, 0.0f);
    mIsPressing = true;
}

void AbstractCursor::setMouseMove(QMouseEvent* aEvent, const CameraInfo& aCameraInfo)
{
    QVector2D prevScreenPos = mScreenPos;
    QVector2D prevWorldPos = mWorldPos;

    mStatus = Status_Move;
    if (!mIsPressing) { mButton = Button_None; }

    mScreenPoint = aEvent->pos();
    mScreenPos = QVector2D(mScreenPoint);
    mScreenVel = mScreenPos - prevScreenPos;
    mWorldPos = aCameraInfo.toWorldPos(mScreenPos);
    mWorldVel = mWorldPos - prevWorldPos;
}

void AbstractCursor::setMouseRelease(QMouseEvent* aEvent, const CameraInfo& aCameraInfo)
{
    QVector2D prevScreenPos = mScreenPos;
    QVector2D prevWorldPos = mWorldPos;

    mStatus = Status_Release;
    mScreenPoint = aEvent->pos();
    mScreenPos = QVector2D(mScreenPoint);
    mScreenVel = mScreenPos - prevScreenPos;
    mWorldPos = aCameraInfo.toWorldPos(mScreenPos);
    mWorldVel = mWorldPos - prevWorldPos;
    mIsPressing = false;
    mIsDouble = false;
}

void AbstractCursor::setMouseDoubleClick(QMouseEvent* aEvent, const CameraInfo& aCameraInfo)
{
    setMousePress(aEvent, aCameraInfo);
    mIsDouble = true;
}

} // namespace core
