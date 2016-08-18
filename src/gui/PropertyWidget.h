#ifndef GUI_PROPERTYWIDGET_H
#define GUI_PROPERTYWIDGET_H

#include <QScrollArea>
#include "util/SlotId.h"
#include "util/LinkPointer.h"
#include "core/Project.h"
#include "core/ObjectNode.h"
#include "core/TimeLineEvent.h"
#include "gui/prop/prop_Backboard.h"

namespace gui
{

class PropertyWidget : public QScrollArea
{
public:
    PropertyWidget(QWidget* aParent);
    ~PropertyWidget();

    void setProject(core::Project* aProject);

    // boostlike signals
    void onSelectionChanged(core::ObjectNode* aRepresentNode);
    void onAttributeUpdated(core::ObjectNode& aNode, bool aUndo);
    void onKeyUpdated(core::TimeLineEvent& aEvent, bool aUndo);
    void onFrameUpdated();
    void onPlayBackStateChanged(bool aIsActive);

    util::Signaler<void()> onVisualUpdated;

private:
    virtual void resizeEvent(QResizeEvent* aEvent);

    void unlinkProject();

    util::LinkPointer<core::Project> mProject;
    util::SlotId mTimeLineSlot;
    util::SlotId mNodeAttrSlot;
    prop::Backboard* mBoard;
};

} // namespace gui

#endif // GUI_PROPERTYWIDGET_H
