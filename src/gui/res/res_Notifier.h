#ifndef GUI_RES_NOTIFIER_H
#define GUI_RES_NOTIFIER_H

#include "cmnd/Listener.h"
#include "img/ResourceNode.h"
#include "core/Project.h"
#include "core/ObjectNode.h"
#include "core/BoneKey.h"
#include "core/ResourceEvent.h"
#include "gui/ViaPoint.h"
#include "gui/res/res_Item.h"

namespace gui {
namespace res {

class ChangeFilePathNotifier : public cmnd::Listener
{
public:
    ChangeFilePathNotifier(ViaPoint& aViaPoint, const img::ResourceNode& aNode);

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
    ViaPoint& mViaPoint;
    const img::ResourceNode& mNode;
};

class ModificationNotifier : public cmnd::Listener
{
public:
    ModificationNotifier(
            ViaPoint& aViaPoint,
            core::Project& aProject,
            const util::TreePos& aRootPos);

    core::ResourceEvent& event() { return mEvent; }
    const core::ResourceEvent& event() const { return mEvent; }

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
    ViaPoint& mViaPoint;
    core::Project& mProject;
    util::TreePos mRootPos;
    core::ResourceEvent mEvent;
};

class AddNewOneNotifier : public cmnd::Listener
{
public:
    AddNewOneNotifier(
            ViaPoint& aViaPoint,
            core::Project& aProject);

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
    ViaPoint& mViaPoint;
    core::Project& mProject;
};

class DeleteNotifier : public cmnd::Listener
{
public:
    DeleteNotifier(
            ViaPoint& aViaPoint,
            core::Project& aProject);

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
    ViaPoint& mViaPoint;
    core::Project& mProject;
};

} // namespace res
} // namespace gui

#endif // GUI_RES_NOTIFIER_H
