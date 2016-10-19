#ifndef CTRL_TIMELINEUTIL_H
#define CTRL_TIMELINEUTIL_H

#include <QVector>
#include "util/Range.h"
#include "cmnd/Stable.h"
#include "cmnd/Listener.h"
#include "core/Project.h"
#include "core/TimeLine.h"
#include "core/TimeLineEvent.h"
#include "core/SRTKey.h"
#include "core/OpaKey.h"
#include "core/BoneKey.h"
#include "core/PoseKey.h"
#include "core/FFDKey.h"
#include "core/MeshKey.h"
#include "core/ImageKey.h"

namespace ctrl
{

namespace TimeLineUtil
{

//-------------------------------------------------------------------------------------------------
class MoveKey : public cmnd::Stable
{
    core::TimeLineEvent& mEvent;
    int mCurrent;

public:
    MoveKey(core::TimeLineEvent& aCommandEvent);
    bool modifyMove(core::TimeLineEvent& aModEvent, int aAdd, const util::Range& aFrame);

    core::TimeLineEvent& event() { return mEvent; }
    const core::TimeLineEvent& event() const { return mEvent; }

    virtual void undo();
    virtual void redo();
};

//-------------------------------------------------------------------------------------------------
class Notifier : public cmnd::Listener
{
public:
    Notifier(core::Project& aProject)
        : mProject(aProject)
        , mEvent()
    {
    }

    core::TimeLineEvent& event() { return mEvent; }
    const core::TimeLineEvent& event() const { return mEvent; }

    virtual void onExecuted()
    {
        mProject.onTimeLineModified(mEvent, false);
    }

    virtual void onUndone()
    {
        mProject.onTimeLineModified(mEvent, true);
    }

    virtual void onRedone()
    {
        mProject.onTimeLineModified(mEvent, false);
    }

private:
    core::Project& mProject;
    core::TimeLineEvent mEvent;
};

//-------------------------------------------------------------------------------------------------
void assignSRTKeyData(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        const core::SRTKey::Data& aNewData);

void assignOpaKeyData(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        const core::OpaKey::Data& aNewData);

void assignPoseKeyEasing(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        const util::Easing::Param& aNewData);

void assignFFDKeyEasing(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        const util::Easing::Param& aNewData);

void pushNewSRTKey(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        core::SRTKey* aKey);

void pushNewOpaKey(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        core::OpaKey* aKey);

void pushNewPoseKey(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        core::PoseKey* aKey, core::BoneKey* aParentKey);

void pushNewFFDKey(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        core::FFDKey* aKey, core::TimeKey* aParentKey);

void pushNewImageKey(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        core::ImageKey* aKey);

//-------------------------------------------------------------------------------------------------
Notifier* createMoveNotifier(core::Project& aProject,
                             core::ObjectNode& aTarget,
                             const core::TimeKeyPos& aPos);

} // namespace TimeLineUtil

} // namespace ctrl

#endif // CTRL_TIMELINEUTIL_H
