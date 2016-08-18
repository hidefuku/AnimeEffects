#ifndef CTRL_BONE_NOTIFIER_H
#define CTRL_BONE_NOTIFIER_H

#include "cmnd/Listener.h"
#include "core/Project.h"
#include "core/ObjectNode.h"
#include "core/BoneKey.h"

namespace ctrl {
namespace bone {

class Notifier : public cmnd::Listener
{
public:
    Notifier(
            core::Project& aProject,
            core::ObjectNode& aTarget,
            core::BoneKey& aKey,
            core::TimeLineEvent::Type aType);

    void notify(bool aIsUndo = false);

    virtual void onExecuted()
    {
        notify();
    }

    virtual void onUndone()
    {
        notify(true);
    }

    virtual void onRedone()
    {
        notify();
    }

private:
    core::Project& mProject;
    core::ObjectNode& mTarget;
    core::BoneKey& mKey;
    core::TimeLineEvent mEvent;

};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_NOTIFIER_H
