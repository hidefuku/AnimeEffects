#include "gui/TargetWidget.h"

namespace gui
{

TargetWidget::TargetWidget(ViaPoint& aViaPoint, GUIResourceSet& aResources, QWidget* aParent, const QSize& aSizeHint)
    : QSplitter(aParent)
    , mProject()
    , mResourceSet(aResources)
    , mSizeHint(aSizeHint)
    , mIsFirstTime(true)
    , mSuspendCount(0)
{
    mObjTree = new ObjectTreeWidget(aViaPoint, aResources, this);
    mTimeLine = new TimeLineWidget(this, *this);
    mPlayBack = new PlayBackWidget(aResources, this);

    this->addWidget(mObjTree);
    this->addWidget(mTimeLine);
    this->addWidget(mPlayBack);
    this->setCollapsible(0, false);
    this->setCollapsible(1, false);
    this->setCollapsible(2, false);

    mPlayBack->setPushDelegate([=](PlayBackWidget::PushType aType)
    {
        this->onPlayBackButtonPushed(aType);
    });
}

void TargetWidget::resizeEvent(QResizeEvent* aEvent)
{
    QSplitter::resizeEvent(aEvent);
    if (mIsFirstTime)
    {
        mIsFirstTime = false;
        this->moveSplitter(this->geometry().width() / 4, 1);
    }
    this->moveSplitter(this->geometry().width() - mPlayBack->constantWidth(), 2);
}

void TargetWidget::setProject(core::Project* aProject)
{
    mPlayBack->pushPauseButton();
    mProject = aProject;
    mObjTree->setProject(aProject);
    mTimeLine->setProject(aProject);
}

core::Frame TargetWidget::currentFrame() const
{
    return mTimeLine->currentFrame();
}

void TargetWidget::stop()
{
    mPlayBack->pushPauseButton();
}

void TargetWidget::suspend()
{
    ++mSuspendCount;
}

void TargetWidget::resume()
{
    XC_ASSERT(mSuspendCount > 0);
    --mSuspendCount;
}

bool TargetWidget::isSuspended() const
{
    return mSuspendCount > 0;
}

void TargetWidget::onPlayBackButtonPushed(PlayBackWidget::PushType aType)
{
    if (!mProject) return;

    if (aType == PlayBackWidget::PushType_Play)
    {
        mTimeLine->setPlayBackActivity(true);
    }
    else if (aType == PlayBackWidget::PushType_Pause)
    {
        mTimeLine->setPlayBackActivity(false);
    }
    else if (aType == PlayBackWidget::PushType_Step)
    {
        mTimeLine->setFrame(currentFrame().added(1));
    }
    else if (aType == PlayBackWidget::PushType_StepBack)
    {
        mTimeLine->setFrame(currentFrame().added(-1));
    }
    else if (aType == PlayBackWidget::PushType_Rewind)
    {
        mTimeLine->setFrame(core::Frame(0));
    }
    else if (aType == PlayBackWidget::PushType_Fast)
    {
        mTimeLine->setFrame(core::Frame(mProject->attribute().maxFrame()));
    }
    else if (aType == PlayBackWidget::PushType_Loop)
    {
        mTimeLine->setPlayBackLoop(true);
    }
    else if (aType == PlayBackWidget::PushType_NoLoop)
    {
        mTimeLine->setPlayBackLoop(false);
    }
}

} // namespace gui
