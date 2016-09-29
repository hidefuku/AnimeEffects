#ifndef GUI_CONTROLWIDGET_H
#define GUI_CONTROLWIDGET_H

#include <QSplitter>
#include "gui/ObjectTreeWidget.h"
#include "gui/TimeLineWidget.h"
#include "gui/GUIResources.h"
#include "gui/PlayBackWidget.h"
#include "gui/ViaPoint.h"
#include "core/Project.h"
#include "core/Animator.h"

namespace gui
{

class TargetWidget
        : public QSplitter
        , public core::Animator
{
public:
    TargetWidget(ViaPoint& aViaPoint, GUIResources& aResources, QWidget* aParent, const QSize& aSizeHint);
    void setProject(core::Project* aProject);

    ObjectTreeWidget& objectTreeWidget() { return *mObjTree; }
    const ObjectTreeWidget& objectTreeWidget() const { return *mObjTree; }

    TimeLineWidget& timeLineWidget() { return *mTimeLine; }
    const TimeLineWidget& timeLineWidget() const { return *mTimeLine; }

    PlayBackWidget& playBackWidget() { return *mPlayBack; }
    const PlayBackWidget& playBackWidget() const { return *mPlayBack; }

    // from Animator
    virtual core::Frame currentFrame() const;
    virtual void stop();
    virtual void suspend();
    virtual void resume();
    virtual bool isSuspended() const;

private:
    virtual void resizeEvent(QResizeEvent* aEvent);
    virtual QSize sizeHint() const { return mSizeHint; }

    void onPlayBackButtonPushed(PlayBackWidget::PushType aType);

    core::Project* mProject;
    GUIResources& mResources;
    const QSize mSizeHint;
    ObjectTreeWidget* mObjTree;
    TimeLineWidget* mTimeLine;
    PlayBackWidget* mPlayBack;
    bool mIsFirstTime;
    int mSuspendCount;
};

} // namespace gui

#endif // GUI_CONTROLWIDGET_H
