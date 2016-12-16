#include "gui/DriverHolder.h"

namespace gui
{

DriverHolder::DriverHolder(ViaPoint& aViaPoint)
    : mViaPoint(aViaPoint)
    , mDriverResources()
    , mDriver()
    , mProject()
    , mTimeLineSlot()
    , mResourceSlot()
    , mTreeSlot()
{
}

DriverHolder::~DriverHolder()
{
    destroy();
}

void DriverHolder::create(core::Project& aProject, ctrl::GraphicStyle& aGraphicStyle)
{
    destroy();

    mProject = aProject.pointee();
    mTimeLineSlot = mProject->onTimeLineModified.connect(this, &DriverHolder::onTimeKeyUpdated);
    mResourceSlot = mProject->onResourceModified.connect(this, &DriverHolder::onResourceUpdated);
    mTreeSlot     = mProject->onTreeRestructured.connect(this, &DriverHolder::onTreeRestructured);

    mDriver.reset(new ctrl::Driver(aProject, mDriverResources, aGraphicStyle, mViaPoint));
    onVisualUpdated();
}

void DriverHolder::destroy()
{
    mDriver.reset();
    if (mProject)
    {
        mProject->onTimeLineModified.disconnect(mTimeLineSlot);
        mProject->onResourceModified.disconnect(mResourceSlot);
        mProject->onTreeRestructured.disconnect(mTreeSlot);
        mProject.reset();
    }
}

void DriverHolder::onFrameUpdated()
{
    if (mDriver)
    {
        mDriver->updateFrame();
        onVisualUpdated();
    }
}

void DriverHolder::onTimeKeyUpdated(core::TimeLineEvent& aEvent, bool aUndo)
{
    if (mDriver)
    {
        mDriver->updateKey(aEvent, aUndo);
        onVisualUpdated();
    }
}

void DriverHolder::onResourceUpdated(core::ResourceEvent& aEvent, bool aUndo)
{
    if (mDriver)
    {
        mDriver->updateResource(aEvent, aUndo);
        onVisualUpdated();
    }
}

void DriverHolder::onTreeRestructured(core::ObjectTreeEvent& aEvent, bool aUndo)
{
    if (mDriver)
    {
        mDriver->updateTree(aEvent, aUndo);
        onVisualUpdated();
    }
}

void DriverHolder::onSelectionChanged(core::ObjectNode* aRepresentNode)
{
    if (mDriver)
    {
        mDriver->setTarget(aRepresentNode);
        onVisualUpdated();
    }
}

void DriverHolder::onProjectAttributeUpdated()
{
    if (mDriver)
    {
        mDriver->updateProjectAttribute();
        onVisualUpdated();
    }
}

} // namespace gui
