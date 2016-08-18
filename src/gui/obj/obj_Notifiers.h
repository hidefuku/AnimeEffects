#ifndef GUI_OBJ_NOTIFIERS_H
#define GUI_OBJ_NOTIFIERS_H

#include "cmnd/Listener.h"
#include "core/ObjectNode.h"
namespace gui { class ObjectTreeWidget; }

namespace gui
{
namespace obj
{

//-------------------------------------------------------------------------------------------------
class ViewUpdatedNotifier : public cmnd::Listener
{
    ObjectTreeWidget& mOwner;
public:
    ViewUpdatedNotifier(ObjectTreeWidget& aOwner);
    virtual void onExecuted();
    virtual void onUndone();
    virtual void onRedone();
};

//-------------------------------------------------------------------------------------------------
class RestructureNotifier : public cmnd::Listener
{
    ObjectTreeWidget& mOwner;
public:
    RestructureNotifier(ObjectTreeWidget& aOwner);
    virtual void onExecuted();
    virtual void onUndone();
    virtual void onRedone();
};

} // namespace obj
} // namespace gui

#endif // GUI_OBJ_NOTIFIERS_H
