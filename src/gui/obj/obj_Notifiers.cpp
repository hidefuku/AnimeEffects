#include "gui/obj/obj_Notifiers.h"
#include "gui/ObjectTreeWidget.h"

namespace gui
{
namespace obj
{

//-------------------------------------------------------------------------------------------------
ViewUpdatedNotifier::ViewUpdatedNotifier(ObjectTreeWidget& aOwner)
    : mOwner(aOwner)
{
}

void ViewUpdatedNotifier::onExecuted()
{
    mOwner.notifyViewUpdated();
}

void ViewUpdatedNotifier::onUndone()
{
    mOwner.notifyViewUpdated();
}

void ViewUpdatedNotifier::onRedone()
{
    mOwner.notifyViewUpdated();
}

//-------------------------------------------------------------------------------------------------
RestructureNotifier::RestructureNotifier(ObjectTreeWidget& aOwner)
    : mOwner(aOwner)
{
}

void RestructureNotifier::onExecuted()
{
    mOwner.notifyRestructure();
}

void RestructureNotifier::onUndone()
{
    mOwner.notifyRestructure();
}

void RestructureNotifier::onRedone()
{
    mOwner.notifyRestructure();
}

} // namespace obj
} // namespace gui
