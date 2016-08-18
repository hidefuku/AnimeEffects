#ifndef CTRL_MESH_NOTIFIER_H
#define CTRL_MESH_NOTIFIER_H

#include "cmnd/Listener.h"
#include "core/Project.h"
#include "core/ObjectNode.h"
#include "core/MeshKey.h"

namespace ctrl {
namespace mesh {

class Notifier : public cmnd::Listener
{
public:
    Notifier(
            core::Project& aProject,
            core::ObjectNode& aTarget,
            core::MeshKey& aKey,
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
    core::MeshKey& mKey;
    core::TimeLineEvent mEvent;

};

} // namespace mesh
} // namespace ctrl

#endif // CTRL_MESH_NOTIFIER_H
