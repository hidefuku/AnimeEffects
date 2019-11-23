#include <QScrollBar>
#include "gui/TimeLineWidget.h"

namespace gui
{

//-------------------------------------------------------------------------------------------------
TimeLineWidget::TimeLineWidget(ViaPoint& aViaPoint, core::Animator& aAnimator, QWidget* aParent)
    : QScrollArea(aParent)
    , mProject()
    , mAnimator(aAnimator)
    , mInner()
    , mCameraInfo()
    , mAbstractCursor()
    , mVerticalScrollValue(0)
    , mTimer()
    , mElapsed()
    , mBeginFrame()
    , mLastFrame()
    , mDoesLoop(false)
{
    mInner = new TimeLineInnerWidget(aViaPoint, this);

    this->setWidget(mInner);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setMouseTracking(true);
    this->connect(&mTimer, &QTimer::timeout, this, &TimeLineWidget::onPlayBackUpdated);
    updateCamera();
}

void TimeLineWidget::setProject(core::Project* aProject)
{
    mProject.reset();
    if (aProject)
    {
        mProject = aProject->pointee();
    }
    mInner->setProject(aProject);
}

void TimeLineWidget::setPlayBackActivity(bool aIsActive)
{
    if (aIsActive)
    {
        mTimer.setInterval(static_cast<int>(getOneFrameTime()));
        mTimer.start();
        mElapsed.start();
        mBeginFrame = currentFrame();
        mLastFrame = mBeginFrame;
    }
    else
    {
        mTimer.stop();
        mBeginFrame.set(0);
        mLastFrame.set(0);
    }
    onPlayBackStateChanged(aIsActive);
}

void TimeLineWidget::setPlayBackLoop(bool aDoesLoop)
{
    mDoesLoop = aDoesLoop;
}

void TimeLineWidget::setFrame(core::Frame aFrame)
{
    mInner->setFrame(aFrame);
    onFrameUpdated();
}

core::Frame TimeLineWidget::currentFrame() const
{
    return mInner->currentFrame();
}

int TimeLineWidget::getFps() const
{
    return mProject ? mProject->attribute().fps() : 60;
}

double TimeLineWidget::getOneFrameTime() const
{
    return 1000.0 / getFps();
}

QPoint TimeLineWidget::viewportTransform() const
{
    // @note bug? Sometimes, the value of vertical scroll bar is different from the set value.

    //return QPoint(-this->horizontalScrollBar()->value(), -this->verticalScrollBar()->value());
    return QPoint(-this->horizontalScrollBar()->value(), -mVerticalScrollValue);
}

void TimeLineWidget::setScrollBarValue(const QPoint& aViewportTransform)
{
    this->horizontalScrollBar()->setValue(-aViewportTransform.x());
    this->verticalScrollBar()->setValue(-aViewportTransform.y());
    mVerticalScrollValue = -aViewportTransform.y();
}

void TimeLineWidget::updateCamera()
{
    mCameraInfo.setScreenWidth(this->rect().width());
    mCameraInfo.setScreenHeight(this->rect().height());
    mCameraInfo.setLeftTopPos(QVector2D(viewportTransform()));
    mCameraInfo.setScale(1.0f);
    mInner->updateCamera(mCameraInfo);
}

void TimeLineWidget::updateCursor(const core::AbstractCursor& aCursor)
{
    onCursorUpdated();
    if (mInner->updateCursor(aCursor))
    {
        onFrameUpdated();
    }
}

//-------------------------------------------------------------------------------------------------
void TimeLineWidget::onTreeViewUpdated(QTreeWidgetItem* aTopNode)
{
    mInner->updateLines(aTopNode);
}

void TimeLineWidget::onScrollUpdated(int aValue)
{
    this->verticalScrollBar()->setValue(aValue);
    mVerticalScrollValue = aValue;
    updateCamera();
}

void TimeLineWidget::onSelectionChanged(core::ObjectNode* aRepresent)
{
    mInner->updateLineSelection(aRepresent);
}

void TimeLineWidget::onPlayBackUpdated()
{
    if (!mAnimator.isSuspended())
    {
        const double oneFrameTime = getOneFrameTime();
        const core::Frame curFrame = currentFrame();
        double nextFrame = static_cast<double>(curFrame.getDecimal()) + 1.0;

        if (mDoesLoop && nextFrame > mInner->maxFrame())
        {
            nextFrame = 0.0;
            mBeginFrame.set(0);
            mElapsed.restart();
            mTimer.setInterval(static_cast<int>(oneFrameTime));
        }
        else
        {
            if (mLastFrame == curFrame)
            {
                const int elapsedTime = mElapsed.elapsed();
                const double elapsedFrame = elapsedTime / oneFrameTime;
                nextFrame = static_cast<double>(mBeginFrame.getDecimal()) + elapsedFrame;

                const double nextUpdateTime = oneFrameTime * (static_cast<int>(elapsedFrame) + 1);
                const double intervalTime = nextUpdateTime - elapsedTime;
                mTimer.setInterval(std::max(static_cast<int>(intervalTime), 1));
            }
            else
            {
                mBeginFrame = curFrame;
                mElapsed.restart();
                mTimer.setInterval(static_cast<int>(oneFrameTime));
            }
        }

#if 0
        setFrame(core::Frame::fromDecimal(nextFrame));
        mLastFrame = core::Frame::fromDecimal(nextFrame);
#else
        setFrame(core::Frame(static_cast<int>(nextFrame)));
        mLastFrame = core::Frame(static_cast<int>(nextFrame));
#endif
    }
}

void TimeLineWidget::onProjectAttributeUpdated()
{
    mInner->updateProjectAttribute();
}

//-------------------------------------------------------------------------------------------------
void TimeLineWidget::mouseMoveEvent(QMouseEvent* aEvent)
{
    QScrollArea::mouseMoveEvent(aEvent);
    if (mAbstractCursor.setMouseMove(aEvent, mCameraInfo))
    {
        updateCursor(mAbstractCursor);
    }
}

void TimeLineWidget::mousePressEvent(QMouseEvent* aEvent)
{
    QScrollArea::mousePressEvent(aEvent);
    if (mAbstractCursor.setMousePress(aEvent, mCameraInfo))
    {
        updateCursor(mAbstractCursor);
    }
}

void TimeLineWidget::mouseReleaseEvent(QMouseEvent* aEvent)
{
    QScrollArea::mouseReleaseEvent(aEvent);
    if (mAbstractCursor.setMouseRelease(aEvent, mCameraInfo))
    {
        updateCursor(mAbstractCursor);
    }
}

void TimeLineWidget::mouseDoubleClickEvent(QMouseEvent* aEvent)
{
    QScrollArea::mouseDoubleClickEvent(aEvent);
    if (mAbstractCursor.setMouseDoubleClick(aEvent, mCameraInfo))
    {
        updateCursor(mAbstractCursor);
    }
}

void TimeLineWidget::wheelEvent(QWheelEvent* aEvent)
{
    QPoint viewTrans = viewportTransform();
    const QPoint cursor = aEvent->pos();
    const QRect rectPrev = mInner->rect();

    mInner->updateWheel(aEvent);

    const QRect rectNext = mInner->rect();
    const double scale = static_cast<double>(rectNext.width() / rectPrev.width());
    viewTrans.setX(static_cast<int>(cursor.x() + scale * (viewTrans.x() - cursor.x())));
    setScrollBarValue(viewTrans);
    updateCamera();
}

void TimeLineWidget::resizeEvent(QResizeEvent* aEvent)
{
    QScrollArea::resizeEvent(aEvent);
    updateCamera();
}

void TimeLineWidget::scrollContentsBy(int aDx, int aDy)
{
    QScrollArea::scrollContentsBy(aDx, aDy);
    updateCamera();
}

} // namespace gui
