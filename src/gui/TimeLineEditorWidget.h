#ifndef GUI_TIMELINEINNERWIDGET_H
#define GUI_TIMELINEINNERWIDGET_H

#include <QWidget>
#include <QAction>
#include <QTreeWidgetItem>
#include "core/TimeLineEvent.h"
#include "ctrl/TimeLineEditor.h"
#include "gui/ViaPoint.h"
#include "gui/GUIResources.h"

namespace gui
{

class TimeCursor : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor DESIGNABLE true)
    Q_PROPERTY(QColor edgeColor READ edgeColor WRITE setEdgeColor DESIGNABLE true)
public:
    TimeCursor(QWidget* aParent);
    void setCursorPos(const QPoint& aPos, int aHeight);
    virtual void paintEvent(QPaintEvent* aEvent);

    QColor bodyColor() const;
    void setBodyColor(const QColor &bodyColor);

    QColor edgeColor() const;
    void setEdgeColor(const QColor &edgeColor);

private:
    QColor mBodyColor;
    QColor mEdgeColor;
};

class TimeLineEditorWidget : public QWidget
{
    Q_OBJECT

    // Properties introduced to let users theme the custom rendered parts of the widget.
    // Qt's stylesheet support for custom rendering is applied per widget via properties.
    Q_PROPERTY(QColor headerContentColor READ headerContentColor WRITE setHeaderContentColor DESIGNABLE true)
    Q_PROPERTY(QColor headerBackgroundColor READ headerBackgroundColor WRITE setHeaderBackgroundColor DESIGNABLE true)

    Q_PROPERTY(QColor trackColor READ trackColor WRITE setTrackColor DESIGNABLE true)
    Q_PROPERTY(QColor trackEdgeColor READ trackEdgeColor WRITE setTrackEdgeColor DESIGNABLE true)
    Q_PROPERTY(QColor trackTextColor READ trackTextColor WRITE setTrackTextColor DESIGNABLE true)
    Q_PROPERTY(QColor trackSelectColor READ trackSelectColor WRITE setTrackSelectColor DESIGNABLE true)
    Q_PROPERTY(QColor trackSeperatorColor READ trackSeperatorColor WRITE setTrackSeperatorColor DESIGNABLE true)

public:
    TimeLineEditorWidget(ViaPoint& aViaPoint, QWidget* aParent);

    void setProject(core::Project* aProject);
    void setFrame(core::Frame aFrame);

    void updateCamera(const core::CameraInfo& aCamera);
    void updateLines(QTreeWidgetItem* aTopNode);
    void updateLineSelection(core::ObjectNode* aRepresent);
    bool updateCursor(const core::AbstractCursor& aCursor);
    void updateWheel(QWheelEvent* aEvent);
    void updateProjectAttribute();
    void updateTheme(theme::Theme&);

    core::Frame currentFrame() const;
    int maxFrame() const;

    QColor headerContentColor() const;
    void setHeaderContentColor(const QColor &headerContentColor);

    QColor headerBackgroundColor() const;
    void setHeaderBackgroundColor(const QColor &headerBackgroundColor);

    QColor trackColor() const;
    void setTrackColor(const QColor &trackColor);

    QColor trackEdgeColor() const;
    void setTrackEdgeColor(const QColor &trackEdgeColor);

    QColor trackTextColor() const;
    void setTrackTextColor(const QColor &trackTextColor);

    QColor trackSelectColor() const;
    void setTrackSelectColor(const QColor &trackSelectColor);

    QColor trackSeperatorColor() const;
    void setTrackSeperatorColor(const QColor &trackSeperatorColor);


private:
    void updateTimeCursorPos();
    void updateSize();
    void updateLinesRecursive(QTreeWidgetItem* aItem);
    virtual void paintEvent(QPaintEvent* aEvent);
    void onTimeLineModified(core::TimeLineEvent& aEvent, bool aUndo);
    void onTreeRestructured(core::ObjectTreeEvent& aEvent, bool aUndo);
    void onProjectAttributeModified(core::ProjectEvent& aEvent, bool aUndo);
    void onContextMenuRequested(const QPoint& aPos);
    void onCopyKeyTriggered(bool);
    void onPasteKeyTriggered(bool);
    void onDeleteKeyTriggered(bool);

    ViaPoint& mViaPoint;
    util::LinkPointer<core::Project> mProject;
    util::SlotId mTimeLineSlot;
    util::SlotId mTreeRestructSlot;
    util::SlotId mProjectAttrSlot;
    QScopedPointer<ctrl::TimeLineEditor> mEditor;
    const core::CameraInfo* mCamera;
    TimeCursor mTimeCursor;

    QAction* mCopyKey;
    QAction* mPasteKey;
    QAction* mDeleteKey;
    core::TimeLineEvent mTargets;
    core::TimeLineEvent mCopyTargets;
    QPoint mPastePos;
    bool mOnPasting;

    theme::TimeLine mTimelineTheme;
};

} // namespace gui

#endif // GUI_TIMELINEINNERWIDGET_H
