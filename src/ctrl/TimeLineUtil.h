#ifndef CTRL_TIMELINEUTIL_H
#define CTRL_TIMELINEUTIL_H

#include <QVector>
#include "util/Range.h"
#include "cmnd/Stable.h"
#include "cmnd/Listener.h"
#include "core/Project.h"
#include "core/TimeLine.h"
#include "core/TimeLineEvent.h"
#include "core/MoveKey.h"
#include "core/RotateKey.h"
#include "core/ScaleKey.h"
#include "core/DepthKey.h"
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
class MoveFrameOfKey : public cmnd::Stable
{
    core::TimeLineEvent& mEvent;
    int mCurrent;

public:
    MoveFrameOfKey(core::TimeLineEvent& aCommandEvent);
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

class ResourceModificationNotifier : public cmnd::Listener
{
public:
    ResourceModificationNotifier(core::Project& aProject)
        : mProject(aProject)
        , mEvent(aProject)
    {
    }

    core::ResourceEvent& event() { return mEvent; }
    const core::ResourceEvent& event() const { return mEvent; }

    virtual void onExecuted()
    {
        mProject.onResourceModified(mEvent, false);
    }

    virtual void onUndone()
    {
        mProject.onResourceModified(mEvent, true);
    }

    virtual void onRedone()
    {
        mProject.onResourceModified(mEvent, false);
    }

private:
    core::Project& mProject;
    core::ResourceEvent mEvent;
};

//-------------------------------------------------------------------------------------------------
void assignDefaultDepthKeyData(
        core::Project& aProject, core::ObjectNode& aTarget,
        const core::DepthKey::Data& aNewData);

void assignDefaultOpaKeyData(
        core::Project& aProject, core::ObjectNode& aTarget,
        const core::OpaKey::Data& aNewData);

//-------------------------------------------------------------------------------------------------
void assignMoveKeyData(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        const core::MoveKey::Data& aNewData);

void assignRotateKeyData(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        const core::RotateKey::Data& aNewData);

void assignScaleKeyData(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        const core::ScaleKey::Data& aNewData);

void assignDepthKeyData(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        const core::DepthKey::Data& aNewData);

void assignOpaKeyData(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        const core::OpaKey::Data& aNewData);

void assignPoseKeyEasing(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        const util::Easing::Param& aNewData);

void assignFFDKeyEasing(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        const util::Easing::Param& aNewData);

void assignImageKeyResource(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        img::ResourceNode& aNewData);

void assignImageKeyOffset(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        const QVector2D& aNewData);

//-------------------------------------------------------------------------------------------------
void pushNewMoveKey(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        core::MoveKey* aKey);

void pushNewRotateKey(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        core::RotateKey* aKey);

void pushNewScaleKey(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        core::ScaleKey* aKey);

void pushNewDepthKey(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        core::DepthKey* aKey);

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
