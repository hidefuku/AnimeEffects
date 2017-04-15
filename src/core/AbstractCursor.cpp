#include "core/AbstractCursor.h"
#include "XC.h"

namespace core
{
AbstractCursor::Button getEventButtonFrom(Qt::MouseButton aButton)
{
    switch (aButton)
    {
    case Qt::LeftButton:  return AbstractCursor::Button_Left;
    case Qt::MidButton:   return AbstractCursor::Button_Middle;
    case Qt::RightButton: return AbstractCursor::Button_Right;
    default:              return AbstractCursor::Button_TERM;
    }
}

//-------------------------------------------------------------------------------------------------
AbstractCursor::AbstractCursor()
    : mEventType(Event_TERM)
    , mEventButton(Button_TERM)
    , mIsPressed()
    , mIsDouble()
    , mScreenPoint()
    , mScreenPos()
    , mScreenVel()
    , mWorldPos()
    , mWorldVel()
    , mPressure(1.0f)
    , mIsPressedTablet()
    , mSuspendedCount(0)
    , mBlankAfterSuspending()
{
    for (int i = 0; i < Button_TERM; ++i)
    {
        mIsPressed[i] = false;
        mIsDouble[i] = false;
        mBlankAfterSuspending[i] = false;
    }
}

bool AbstractCursor::setMousePress(QMouseEvent* aEvent, const CameraInfo& aCameraInfo)
{
    return setMousePressImpl(aEvent->button(), aEvent->pos(), aCameraInfo);
}

bool AbstractCursor::setMousePressImpl(Qt::MouseButton aButton, QPoint aPos, const CameraInfo& aCameraInfo)
{
    //qDebug() << "press" << aButton;
    mEventType = Event_Press;
    mEventButton = getEventButtonFrom(aButton);

    if (mEventButton != Button_TERM) // fail safe code
    {
        if (mIsPressed[mEventButton]) // duplicate event?
        {
            mEventType = Event_TERM;
            mEventButton = Button_TERM;
            return false;
        }
        mIsPressed[mEventButton] = true;
    }

    mScreenPoint = aPos;
    mScreenPos = QVector2D(mScreenPoint);
    mScreenVel = QVector2D(0.0f, 0.0f);
    mWorldPos = aCameraInfo.toWorldPos(mScreenPos);
    mWorldVel = QVector2D(0.0f, 0.0f);

    return mSuspendedCount == 0;
}

bool AbstractCursor::setMouseMove(QMouseEvent* aEvent, const CameraInfo& aCameraInfo)
{
    return setMouseMoveImpl(aEvent->button(), aEvent->pos(), aCameraInfo);
}

bool AbstractCursor::setMouseMoveImpl(Qt::MouseButton, QPoint aPos, const CameraInfo& aCameraInfo)
{
    QVector2D prevScreenPos = mScreenPos;
    QVector2D prevWorldPos = mWorldPos;

    mEventType = Event_Move;
    mEventButton = Button_TERM;

    mScreenPoint = aPos;
    mScreenPos = QVector2D(mScreenPoint);
    mScreenVel = mScreenPos - prevScreenPos;
    mWorldPos = aCameraInfo.toWorldPos(mScreenPos);
    mWorldVel = mWorldPos - prevWorldPos;

    return mSuspendedCount == 0;
}

bool AbstractCursor::setMouseRelease(QMouseEvent* aEvent, const CameraInfo& aCameraInfo)
{
    return setMouseReleaseImpl(aEvent->button(), aEvent->pos(), aCameraInfo);
}

bool AbstractCursor::setMouseReleaseImpl(Qt::MouseButton aButton, QPoint aPos, const CameraInfo& aCameraInfo)
{
    //qDebug() << "release" << aButton;
    QVector2D prevScreenPos = mScreenPos;
    QVector2D prevWorldPos = mWorldPos;

    mEventType = Event_Release;
    mEventButton = getEventButtonFrom(aButton);

    bool isDuplicateEvent = false;
    if (mEventButton != Button_TERM) // fail safe code
    {
        if (!mIsPressed[mEventButton]) // duplicate event?
        {
            isDuplicateEvent = true;
        }
        mIsPressed[mEventButton] = false;
        mIsDouble[mEventButton] = false;
    }

    mScreenPoint = aPos;
    mScreenPos = QVector2D(mScreenPoint);
    mScreenVel = mScreenPos - prevScreenPos;
    mWorldPos = aCameraInfo.toWorldPos(mScreenPos);
    mWorldVel = mWorldPos - prevWorldPos;

    bool shouldUpdate = true;
    if (mSuspendedCount > 0)
    {
        shouldUpdate = false;
    }
    else if (mEventButton != Button_TERM && mBlankAfterSuspending.at(mEventButton))
    {
        mBlankAfterSuspending.at(mEventButton) = false;
        shouldUpdate = false;
    }
    else if (isDuplicateEvent)
    {
        shouldUpdate = false;
    }

    if (!shouldUpdate)
    {
        mEventType = Event_TERM;
        mEventButton = Button_TERM;
    }

    return shouldUpdate;
}

bool AbstractCursor::setMouseDoubleClick(QMouseEvent* aEvent, const CameraInfo& aCameraInfo)
{
    setMousePress(aEvent, aCameraInfo);

    if (mEventButton != Button_TERM) // fail safe code
    {
        mIsDouble[mEventButton] = true;
    }

    return mSuspendedCount == 0;
}

#ifdef Q_OS_MAC
bool AbstractCursor::setTabledEvent(QTabletEvent* aEvent, const CameraInfo& aCameraInfo)
{
    auto type = aEvent->type();
    auto pressure = aEvent->pressure();

    bool shouldUpdate = false;
    if (type == QEvent::TabletPress)
    {
        shouldUpdate = setMousePressImpl(aEvent->button(), aEvent->pos(), aCameraInfo);
        mIsPressedTablet = true;
        mPressure = pressure;
    }
    else if (type == QEvent::TabletMove)
    {
        shouldUpdate = setMouseMoveImpl(aEvent->button(), aEvent->pos(), aCameraInfo);
        if (mIsPressedTablet)
        {
            mPressure = 0.5f * mPressure + 0.5f * pressure;
        }
    }
    else if (type == QEvent::TabletRelease)
    {
        shouldUpdate = setMouseReleaseImpl(aEvent->button(), aEvent->pos(), aCameraInfo);
        mIsPressedTablet = false;
        mPressure = 1.0f;
    }

    return shouldUpdate;
}
#else
void AbstractCursor::setTabletPressure(QTabletEvent* aEvent)
{
    auto type = aEvent->type();
    auto pressure = aEvent->pressure();

    if (type == QEvent::TabletPress)
    {
        mIsPressedTablet = true;
        mPressure = pressure;
    }
    else if (type == QEvent::TabletMove)
    {
        if (mIsPressedTablet)
        {
            mPressure = 0.5f * mPressure + 0.5f * pressure;
        }
    }
    else if (type == QEvent::TabletRelease)
    {
        mIsPressedTablet = false;
        mPressure = 1.0f;
    }
}
#endif

void AbstractCursor::suspendEvent(const std::function<void()>& aEventReflector)
{
    XC_ASSERT(mSuspendedCount >= 0);
    ++mSuspendedCount;
    if (mSuspendedCount == 1)
    {
        for (int i = 0; i < Button_TERM; ++i)
        {
            if (mIsPressed[i]) // invoke a release event
            {
                mEventType = Event_Release;
                mEventButton = (Button)i;

                mIsPressed[i] = false;
                mScreenVel = QVector2D();
                mWorldVel = QVector2D();

                aEventReflector();
            }
        }
    }
}

void AbstractCursor::resumeEvent()
{
    XC_ASSERT(mSuspendedCount > 0);
    --mSuspendedCount;

    if (mSuspendedCount == 0)
    {
        for (int i = 0; i < Button_TERM; ++i)
        {
            mBlankAfterSuspending[i] = mIsPressed[i]; // set a flag to ignore release event once
        }
    }
}

bool AbstractCursor::emitsLeftPressedEvent() const
{
    return mEventType == Event_Press && mEventButton == Button_Left;
}

bool AbstractCursor::emitsLeftDraggedEvent() const
{
    return mEventType == Event_Move && mIsPressed[Button_Left] && !mBlankAfterSuspending.at(Button_Left);
}

bool AbstractCursor::emitsLeftReleasedEvent() const
{
    return mEventType == Event_Release && mEventButton == Button_Left;
}

bool AbstractCursor::emitsRightPressedEvent() const
{
    return mEventType == Event_Press && mEventButton == Button_Right;
}

bool AbstractCursor::emitsRightDraggedEvent() const
{
    return mEventType == Event_Move && mIsPressed[Button_Right] && !mBlankAfterSuspending.at(Button_Right);
}

bool AbstractCursor::emitsRightReleasedEvent() const
{
    return mEventType == Event_Release && mEventButton == Button_Right;
}

bool AbstractCursor::emitsPressedEvent() const
{
    return mEventType == Event_Press;
}

} // namespace core
