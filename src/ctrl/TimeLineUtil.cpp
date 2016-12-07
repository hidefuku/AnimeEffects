#include <QDebug>
#include "cmnd/BasicCommands.h"
#include "cmnd/ScopedMacro.h"
#include "core/TimeKeyType.h"
#include "core/ImageKeyUpdater.h"
#include "core/FFDKeyUpdater.h"
#include "core/ResourceUpdatingWorkspace.h"
#include "ctrl/TimeLineUtil.h"

using namespace core;

namespace ctrl
{

namespace TimeLineUtil
{

//---------------------------------------------------------------------------------------
MoveFrameOfKey::MoveFrameOfKey(TimeLineEvent& aCommandEvent)
    : mEvent(aCommandEvent)
    , mCurrent(0)
{
}

bool MoveFrameOfKey::modifyMove(TimeLineEvent& aModEvent, int aAdd, const util::Range& aFrame)
{
    aModEvent.setType(TimeLineEvent::Type_MoveKey);

    // empty
    if (aAdd == 0)
    {
        return true;
    }

    // if next frames has a invalid value, nothing to do
    for (const TimeLineEvent::Target& target : mEvent.targets())
    {
        XC_ASSERT(!target.pos.isNull());

        int current = 0;
        int next = 0;

        if (mCurrent == 0)
        {
            current = target.pos.index();
            next = target.subIndex + aAdd;
        }
        else
        {
            current = target.subIndex;
            next = target.subIndex + aAdd;
        }

        if (next < aFrame.min() || aFrame.max() < next) return false;

        if (!target.pos.map().contains(current)) return false;

        if (target.pos.map().contains(next)) return false;
    }

    if (mCurrent == 0)
    {
        // this command is not executed
        for (TimeLineEvent::Target& target : mEvent.targets())
        {
            target.subIndex += aAdd;
        }
    }
    else
    {
        // this command was already executed
        for (TimeLineEvent::Target& target : mEvent.targets())
        {
            const int curr = target.subIndex;
            const int next = target.subIndex + aAdd;

            target.subIndex = next;
            aModEvent.pushTarget(*target.node, target.pos.type(), curr, next);

            // move target
            target.pos.line()->move(target.pos.type(), curr, next);
        }
    }
    return true;
}

void MoveFrameOfKey::undo()
{
    for (TimeLineEvent::Target& target : mEvent.targets())
    {
        if (target.pos.index() == target.subIndex) continue;

        // move
        const int curr = target.subIndex;
        const int next = target.pos.index();
        const bool success = target.pos.line()->move(target.pos.type(), curr, next);
        XC_ASSERT(success); (void)success;
    }

    mCurrent = 0;
}

void MoveFrameOfKey::redo()
{
    for (TimeLineEvent::Target& target : mEvent.targets())
    {
        if (target.pos.index() == target.subIndex) continue;

        // move
        const int curr = target.pos.index();
        const int next = target.subIndex;
        bool success = target.pos.line()->move(target.pos.type(), curr, next);
        XC_ASSERT(success); (void)success;
    }

    mCurrent = 1;
}

//---------------------------------------------------------------------------------------
template<class tKey, TimeKeyType tType>
void assignKeyData(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const typename tKey::Data& aNewData, const QString& aText)
{
    XC_ASSERT(aTarget.timeLine());
    tKey* key = (tKey*)(aFrame == TimeLine::kDefaultKeyIndex ?
                            aTarget.timeLine()->defaultKey(tType) :
                            aTarget.timeLine()->timeKey(tType, aFrame));
    XC_PTR_ASSERT(key);

    {
        cmnd::ScopedMacro macro(aProject.commandStack(), aText);

        auto notifier = new Notifier(aProject);
        notifier->event().setType(TimeLineEvent::Type_ChangeKeyValue);
        if (aFrame == TimeLine::kDefaultKeyIndex)
        {
            notifier->event().pushDefaultTarget(aTarget, tType);
        }
        else
        {
            notifier->event().pushTarget(aTarget, tType, aFrame);
        }
        macro.grabListener(notifier);

        auto command = new cmnd::Assign<typename tKey::Data>(&(key->data()), aNewData);
        aProject.commandStack().push(command);
    }
}

//---------------------------------------------------------------------------------------
template<class tKey, TimeKeyType tType>
void assignKeyEasing(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const util::Easing::Param& aNewData, const QString& aText)
{
    XC_ASSERT(aTarget.timeLine());
    tKey* key = (tKey*)(aFrame == TimeLine::kDefaultKeyIndex ?
                            aTarget.timeLine()->defaultKey(tType) :
                            aTarget.timeLine()->timeKey(tType, aFrame));
    XC_PTR_ASSERT(key);

    {
        cmnd::ScopedMacro macro(aProject.commandStack(), aText);

        auto notifier = new Notifier(aProject);
        notifier->event().setType(TimeLineEvent::Type_ChangeKeyValue);
        if (aFrame == TimeLine::kDefaultKeyIndex)
        {
            notifier->event().pushDefaultTarget(aTarget, tType);
        }
        else
        {
            notifier->event().pushTarget(aTarget, tType, aFrame);
        }
        macro.grabListener(notifier);

        auto command = new cmnd::Assign<util::Easing::Param>(&(key->data().easing()), aNewData);
        aProject.commandStack().push(command);
    }
}

//---------------------------------------------------------------------------------------
template<class tKey, TimeKeyType tType>
void assignKeyBy(
        Project& aProject, ObjectNode& aTarget, int aFrame, const QString& aText,
        const std::function<void(tKey*)>& aFunc)
{
    XC_ASSERT(aTarget.timeLine());
    tKey* key = (tKey*)(aFrame == TimeLine::kDefaultKeyIndex ?
                            aTarget.timeLine()->defaultKey(tType) :
                            aTarget.timeLine()->timeKey(tType, aFrame));
    XC_PTR_ASSERT(key);

    {
        cmnd::ScopedMacro macro(aProject.commandStack(), aText);

        auto notifier = new Notifier(aProject);
        notifier->event().setType(TimeLineEvent::Type_ChangeKeyValue);
        if (aFrame == TimeLine::kDefaultKeyIndex)
        {
            notifier->event().pushDefaultTarget(aTarget, tType);
        }
        else
        {
            notifier->event().pushTarget(aTarget, tType, aFrame);
        }
        macro.grabListener(notifier);

        aFunc(key);
    }
}

//---------------------------------------------------------------------------------------
void assignMoveKeyData(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const MoveKey::Data& aNewData)
{
    assignKeyData<MoveKey, TimeKeyType_Move>(
                aProject, aTarget, aFrame, aNewData,
                aFrame == TimeLine::kDefaultKeyIndex ?
                    "assign default move" : "assign move key");
}

void assignRotateKeyData(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const RotateKey::Data& aNewData)
{
    assignKeyData<RotateKey, TimeKeyType_Rotate>(
                aProject, aTarget, aFrame, aNewData,
                aFrame == TimeLine::kDefaultKeyIndex ?
                    "assign default rotate" : "assign rotate key");
}

void assignScaleKeyData(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const ScaleKey::Data& aNewData)
{
    assignKeyData<ScaleKey, TimeKeyType_Scale>(
                aProject, aTarget, aFrame, aNewData,
                aFrame == TimeLine::kDefaultKeyIndex ?
                "assign default scale" : "assign scale key");
}

void assignDepthKeyData(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const DepthKey::Data& aNewData)
{
    assignKeyData<DepthKey, TimeKeyType_Depth>(
                aProject, aTarget, aFrame, aNewData,
                aFrame == TimeLine::kDefaultKeyIndex ?
                "assign default depth" : "assign depth key");
}

void assignOpaKeyData(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const OpaKey::Data& aNewData)
{
    assignKeyData<OpaKey, TimeKeyType_Opa>(
                aProject, aTarget, aFrame, aNewData,
                aFrame == TimeLine::kDefaultKeyIndex ?
                "assign default opacity" : "assign opacity key");
}

void assignPoseKeyEasing(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const util::Easing::Param& aNewData)
{
    assignKeyEasing<PoseKey, TimeKeyType_Pose>(
                aProject, aTarget, aFrame, aNewData,
                aFrame == TimeLine::kDefaultKeyIndex ?
                "assign default pose" : "assign pose key");
}

void assignFFDKeyEasing(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const util::Easing::Param& aNewData)
{
    assignKeyEasing<FFDKey, TimeKeyType_FFD>(
                aProject, aTarget, aFrame, aNewData,
                aFrame == TimeLine::kDefaultKeyIndex ?
                "assign default ffd" : "assign ffd key");
}

void assignImageKeyResource(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        img::ResourceNode& aNewData)
{
    ResourceUpdatingWorkspacePtr workspace = std::make_shared<ResourceUpdatingWorkspace>();
    const bool createTransitions = !aTarget.timeLine()->isEmpty(TimeKeyType_FFD);

    assignKeyBy<ImageKey, TimeKeyType_Image>(
                aProject, aTarget, aFrame,
                aFrame == TimeLine::kDefaultKeyIndex ?
                "assign default image resource" : "assign image key resource",
                [&](ImageKey* aKey)
    {
        // image key
        aProject.commandStack().push(
                    ImageKeyUpdater::createResourceUpdater(
                        *aKey, aNewData, workspace, createTransitions));

        // ffd key should be called finally
        if (createTransitions)
        {
            aProject.commandStack().push(
                        FFDKeyUpdater::createResourceUpdater(aTarget, workspace));
        }
    });
}

void assignImageKeyOffset(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const QVector2D& aNewData)
{
    assignKeyBy<ImageKey, TimeKeyType_Image>(
                aProject, aTarget, aFrame,
                aFrame == TimeLine::kDefaultKeyIndex ?
                "assign default image center" : "assign image key center",
                [&](ImageKey* aKey)
    {
        auto prevOffset = aKey->data().imageOffset();

        aProject.commandStack().push(new cmnd::Delegatable([=]()
        {
            aKey->data().setImageOffset(aNewData);
        },
        [=]()
        {
            aKey->data().setImageOffset(prevOffset);
        }));
    });
}

void assignImageKeyCellSize(
        core::Project& aProject, core::ObjectNode& aTarget, int aFrame,
        int aNewData)
{
    ResourceUpdatingWorkspacePtr workspace = std::make_shared<ResourceUpdatingWorkspace>();
    const bool createTransitions = !aTarget.timeLine()->isEmpty(TimeKeyType_FFD);

    assignKeyBy<ImageKey, TimeKeyType_Image>(
                aProject, aTarget, aFrame,
                aFrame == TimeLine::kDefaultKeyIndex ?
                "assign default image cell size" : "assign image key cell size",
                [&](ImageKey* aKey)
    {
        // image key
        aProject.commandStack().push(
                    ImageKeyUpdater::createGridMeshUpdater(
                        *aKey, aNewData, workspace, createTransitions));

        // ffd key should be called finally
        if (createTransitions)
        {
            aProject.commandStack().push(
                        FFDKeyUpdater::createResourceUpdater(aTarget, workspace));
        }
    });
}

//---------------------------------------------------------------------------------------
template<class tKey, TimeKeyType tType>
void pushNewKey(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        tKey* aKey, const QString& aText, TimeKey* aParentKey = nullptr)
{
    XC_PTR_ASSERT(aKey);
    XC_ASSERT(aTarget.timeLine());

    {
        cmnd::ScopedMacro macro(aProject.commandStack(), aText);

        // set notifier
        auto notifier = new Notifier(aProject);
        notifier->event().setType(TimeLineEvent::Type_PushKey);
        notifier->event().pushTarget(aTarget, tType, aFrame);
        macro.grabListener(notifier);

        // create commands
        auto& stack = aProject.commandStack();
        stack.push(new cmnd::GrabNewObject<tKey>(aKey));
        stack.push(aTarget.timeLine()->createPusher(tType, aFrame, aKey));

        if (aParentKey)
        {
            stack.push(new cmnd::PushBackTree<TimeKey>(&aParentKey->children(), aKey));
        }
    }
}

void pushNewMoveKey(
        Project& aProject, ObjectNode& aTarget, int aFrame, MoveKey* aKey)
{
    pushNewKey<MoveKey, TimeKeyType_Move>(
                aProject, aTarget, aFrame, aKey, "push new move key");
}
void pushNewRotateKey(
        Project& aProject, ObjectNode& aTarget, int aFrame, RotateKey* aKey)
{
    pushNewKey<RotateKey, TimeKeyType_Rotate>(
                aProject, aTarget, aFrame, aKey, "push new rotate key");
}

void pushNewScaleKey(
        Project& aProject, ObjectNode& aTarget, int aFrame, ScaleKey* aKey)
{
    pushNewKey<ScaleKey, TimeKeyType_Scale>(
                aProject, aTarget, aFrame, aKey, "push new scale key");
}

void pushNewDepthKey(
        Project& aProject, ObjectNode& aTarget, int aFrame, DepthKey* aKey)
{
    pushNewKey<DepthKey, TimeKeyType_Depth>(
                aProject, aTarget, aFrame, aKey, "push new depth key");
}

void pushNewOpaKey(
        Project& aProject, ObjectNode& aTarget, int aFrame, OpaKey* aKey)
{
    pushNewKey<OpaKey, TimeKeyType_Opa>(
                aProject, aTarget, aFrame, aKey, "push new opacity key");
}

void pushNewPoseKey(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        PoseKey* aKey, BoneKey* aParentKey)
{
    XC_PTR_ASSERT(aParentKey);
    pushNewKey<PoseKey, TimeKeyType_Pose>(
                aProject, aTarget, aFrame, aKey,
                "push new pose key", aParentKey);
}

void pushNewFFDKey(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        FFDKey* aKey, TimeKey* aParentKey)
{
    XC_ASSERT(!aParentKey ||
              aParentKey->type() == TimeKeyType_Mesh ||
              aParentKey->type() == TimeKeyType_Image);
    pushNewKey<FFDKey, TimeKeyType_FFD>(
                aProject, aTarget, aFrame, aKey,
                "push new ffd key", aParentKey);
}

void pushNewImageKey(
        Project& aProject, ObjectNode& aTarget, int aFrame, ImageKey* aKey)
{
    pushNewKey<ImageKey, TimeKeyType_Image>(
                aProject, aTarget, aFrame, aKey, "push new image key");
}

//-------------------------------------------------------------------------------------------------
Notifier* createMoveNotifier(Project& aProject,
                             ObjectNode& aTarget,
                             const TimeKeyPos& aPos)
{
    auto notifier = new Notifier(aProject);
    notifier->event().setType(TimeLineEvent::Type_MoveKey);
    notifier->event().pushTarget(aTarget, aPos);
    return notifier;
}

} // namespace TimeLineUtil

} // namespace ctrl
