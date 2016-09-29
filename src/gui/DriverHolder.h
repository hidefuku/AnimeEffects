#ifndef GUI_DRIVERHOLDER_H
#define GUI_DRIVERHOLDER_H

#include "util/LinkPointer.h"
#include "util/Signaler.h"
#include "core/Project.h"
#include "ctrl/System.h"
#include "ctrl/Driver.h"
#include "ctrl/ToolType.h"
#include "ctrl/DriverResources.h"
#include "ctrl/GraphicStyle.h"

namespace gui
{

class DriverHolder
{
public:
    DriverHolder();
    ~DriverHolder();

    void create(core::Project& aProject, ctrl::GraphicStyle& aGraphicStyle);
    void destroy();

    ctrl::Driver* driver() { return mDriver.data(); }
    const ctrl::Driver* driver() const { return mDriver.data(); }

    // boostlike signals
public:
    util::Signaler<void()> onVisualUpdated;

    void onFrameUpdated();
    void onTimeKeyUpdated(core::TimeLineEvent& aEvent, bool aUndo);
    void onResourceUpdated(core::ResourceEvent& aEvent, bool aUndo);
    void onTreeRestructured(core::ObjectTreeEvent& aEvent, bool aUndo);
    void onSelectionChanged(core::ObjectNode* aRepresentNode);
    void onProjectAttributeUpdated();

private:

    ctrl::DriverResources mDriverResources;
    QScopedPointer<ctrl::Driver> mDriver;
    util::LinkPointer<core::Project> mProject;
    util::SlotId mTimeLineSlot;
    util::SlotId mResourceSlot;
    util::SlotId mTreeSlot;
};

} // namespace gui

#endif // GUI_DRIVERHOLDER_H
