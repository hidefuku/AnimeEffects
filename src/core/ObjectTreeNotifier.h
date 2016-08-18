#ifndef CORE_OBJECTTREENOTIFIER_H
#define CORE_OBJECTTREENOTIFIER_H

#include <QVector>
#include "cmnd/Listener.h"
#include "core/ObjectTreeEvent.h"
namespace core { class Project; }

namespace core
{

class ObjectTreeNotifier : public cmnd::Listener
{
public:
    ObjectTreeNotifier(Project& aProject);

    ObjectTreeEvent& event() { return mEvent; }
    const ObjectTreeEvent& event() const { return mEvent; }

    virtual void onExecuted();
    virtual void onUndone();
    virtual void onRedone();

private:
    ObjectTreeEvent mEvent;
};

} // namespace core

#endif // CORE_OBJECTTREENOTIFIER_H
