#ifndef GUI_TIMELINEWIDGET_H
#define GUI_TIMELINEWIDGET_H

#include <QScrollArea>
#include <QTreeWidgetItem>
#include <QTimer>
#include <QTime>
#include <QScopedPointer>
#include "util/Signaler.h"
#include "core/Project.h"
#include "core/CameraInfo.h"
#include "core/AbstractCursor.h"
#include "core/Animator.h"
#include "core/TimeLineEvent.h"
#include "ctrl/TimeLineEditor.h"
#include "gui/TimeLineInnerWidget.h"
#include "gui/ViaPoint.h"

namespace gui
{

class TimeLineWidget : public QScrollArea
{
    Q_OBJECT
public:
    //typedef std::function<void(const core::TimeInfo&)> PlayBackFunc;

    TimeLineWidget(ViaPoint& aViaPoint, core::Animator& aAnimator, QWidget* aParent);

    void setProject(core::Project* aProject);
    void updateLines(QTreeWidgetItem* aTopNode);
    void setPlayBackActivity(bool aIsActive);
    void setPlayBackLoop(bool aDoesLoop);
    void setFrame(core::Frame aFrame);
    core::Frame currentFrame() const;

    util::Signaler<void()> onCursorUpdated;
    util::Signaler<void()> onFrameUpdated;
    util::Signaler<void(bool)> onPlayBackStateChanged;

public:
    void onTreeViewUpdated(QTreeWidgetItem* aTopNode);
    void onScrollUpdated(int aValue);
    void onSelectionChanged(core::ObjectNode* aRepresent);
    void onProjectAttributeUpdated();

private:
    virtual void mouseMoveEvent(QMouseEvent* aEvent);
    virtual void mousePressEvent(QMouseEvent* aEvent);
    virtual void mouseReleaseEvent(QMouseEvent* aEvent);
    virtual void mouseDoubleClickEvent(QMouseEvent* aEvent);
    virtual void wheelEvent(QWheelEvent* aEvent);
    virtual void resizeEvent(QResizeEvent* aEvent);
    virtual void scrollContentsBy(int aDx, int aDy);

    int getFps() const;
    double getOneFrameTime() const;
    QPoint viewportTransform() const;
    void setScrollBarValue(const QPoint& aViewportTransform);
    void updateCamera();
    void updateCursor(const core::AbstractCursor& aCursor);
    void onPlayBackUpdated();

    util::LinkPointer<core::Project> mProject;
    core::Animator& mAnimator;
    TimeLineInnerWidget* mInner;
    core::CameraInfo mCameraInfo;
    core::AbstractCursor mAbstractCursor;
    int mVerticalScrollValue;

    // for animation
    QTimer mTimer;
    QTime mElapsed;
    core::Frame mBeginFrame;
    core::Frame mLastFrame;
    bool mDoesLoop;
};

} // namespace gui

#endif // GUI_TIMELINEWIDGET_H
