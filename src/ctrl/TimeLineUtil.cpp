#include <QDebug>
#include "cmnd/BasicCommands.h"
#include "cmnd/ScopedMacro.h"
#include "core/TimeKeyType.h"
#include "core/ImageKeyUpdater.h"
#include "core/FFDKeyUpdater.h"
#include "core/ResourceUpdatingWorkspace.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/CmndName.h"

using namespace core;

namespace ctrl
{

namespace TimeLineUtil
{

//---------------------------------------------------------------------------------------
MoveFrameOfKey::MoveFrameOfKey(const TimeLineEvent& aCommandEvent)
    : mSortedTargets()
    , mCurrent(0)
    , mMove(0)
{
    for (const TimeLineEvent::Target& target : aCommandEvent.targets())
    {
        // check validity of initial values
        XC_ASSERT(target.pos.index() == target.subIndex);

        mSortedTargets.push_back(target);
    }
    // sort targets for the purpose of the move with no conflict.
    std::sort(mSortedTargets.begin(), mSortedTargets.end(), lessThan);
}

bool MoveFrameOfKey::lessThan(const TimeLineEvent::Target& aLhs, const TimeLineEvent::Target& aRhs)
{
    return aLhs.pos.index() < aRhs.pos.index();
}

bool MoveFrameOfKey::contains(const core::TimeLine::MapType& aMap, int aIndex)
{
    if (mCurrent == 0)
    {
        for (const TimeLineEvent::Target& target : mSortedTargets)
        {
            if (&target.pos.map() == &aMap && target.pos.index() == aIndex) return true;
        }
    }
    else
    {
        for (const TimeLineEvent::Target& target : mSortedTargets)
        {
            if (&target.pos.map() == &aMap && target.subIndex == aIndex) return true;
        }
    }
    return false;
}

bool MoveFrameOfKey::modifyMove(TimeLineEvent& aModEvent, int aAdd,
                                const util::Range& aFrame, int* aClampedAdd)
{
    aModEvent.setType(TimeLineEvent::Type_MoveKey);

    if (aClampedAdd) *aClampedAdd = aAdd;
    if (aAdd == 0) return true; // empty

    // clamp aAdd value by the frame range
    for (const TimeLineEvent::Target& target : mSortedTargets)
    {
        XC_ASSERT(!target.pos.isNull());
        const int next = target.subIndex + aAdd;
        aAdd += xc_clamp(next, aFrame.min(), aFrame.max()) - next;
    }
    if (aClampedAdd) *aClampedAdd = aAdd;
    if (aAdd == 0) return true;

    // if next frames has a invalid value, nothing to do
    for (const TimeLineEvent::Target& target : mSortedTargets)
    {
        XC_ASSERT(!target.pos.isNull());

        const int current = (mCurrent == 0) ? target.pos.index() : target.subIndex;
        const int next = target.subIndex + aAdd;

        // check out of range (fail safe code)
        if (next < aFrame.min() || aFrame.max() < next) return false;

        // key isn't exist
        if (!target.pos.map().contains(current)) return false;

        // conflict with other key
        if (target.pos.map().contains(next))
        {
            if (!contains(target.pos.map(), next)) return false;
        }
    }

    // update move
    mMove += aAdd;

    if (mCurrent == 0) // this command is not executed
    {
        for (TimeLineEvent::Target& target : mSortedTargets)
        {
            target.subIndex += aAdd;
        }
    }
    else // this command was already executed
    {
        // setup mod event
        for (TimeLineEvent::Target& target : mSortedTargets)
        {
            aModEvent.pushTarget(*target.node, target.pos.type(),
                                 target.subIndex, target.subIndex + aAdd);
        }

        // move all (we have to avoid conflict)
        if (aAdd < 0)
        {
            for (TimeLineEvent::Target& target : mSortedTargets)
            {
                auto success = target.pos.line()->move(
                            target.pos.type(), target.subIndex, target.subIndex + aAdd);
                XC_ASSERT(success); (void)success;
                target.subIndex += aAdd;
            }
        }
        else if (aAdd > 0)
        {
            for (auto itr = mSortedTargets.rbegin(); itr != mSortedTargets.rend(); ++itr)
            {
                auto& target = *itr;
                auto success = target.pos.line()->move(
                            target.pos.type(), target.subIndex, target.subIndex + aAdd);
                XC_ASSERT(success); (void)success;
                target.subIndex += aAdd;
            }
        }
    }
    return true;
}

void MoveFrameOfKey::undo()
{
    if (mMove < 0)
    {
        for (auto itr = mSortedTargets.rbegin(); itr != mSortedTargets.rend(); ++itr)
        {
            auto& target = *itr;
            auto success = target.pos.line()->move(
                        target.pos.type(), target.subIndex, target.pos.index());
            XC_ASSERT(success); (void)success;
        }
    }
    else if (mMove > 0)
    {
        for (TimeLineEvent::Target& target : mSortedTargets)
        {
            auto success = target.pos.line()->move(
                        target.pos.type(), target.subIndex, target.pos.index());
            XC_ASSERT(success); (void)success;
        }
    }

    mCurrent = 0;
}

void MoveFrameOfKey::redo()
{
    if (mMove < 0)
    {
        for (TimeLineEvent::Target& target : mSortedTargets)
        {
            auto success = target.pos.line()->move(
                        target.pos.type(), target.pos.index(), target.subIndex);
            XC_ASSERT(success); (void)success;
        }
    }
    else if (mMove > 0)
    {
        for (auto itr = mSortedTargets.rbegin(); itr != mSortedTargets.rend(); ++itr)
        {
            auto& target = *itr;
            auto success = target.pos.line()->move(
                        target.pos.type(), target.pos.index(), target.subIndex);
            XC_ASSERT(success); (void)success;
        }
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
                    CmndName::tr("update a default movement") :
                    CmndName::tr("update a movement key"));
}

void assignRotateKeyData(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const RotateKey::Data& aNewData)
{
    assignKeyData<RotateKey, TimeKeyType_Rotate>(
                aProject, aTarget, aFrame, aNewData,
                aFrame == TimeLine::kDefaultKeyIndex ?
                    CmndName::tr("update a default rotation") :
                    CmndName::tr("update a rotation key"));
}

void assignScaleKeyData(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const ScaleKey::Data& aNewData)
{
    assignKeyData<ScaleKey, TimeKeyType_Scale>(
                aProject, aTarget, aFrame, aNewData,
                aFrame == TimeLine::kDefaultKeyIndex ?
                    CmndName::tr("update a default scaling") :
                    CmndName::tr("update a scaling key"));
}

void assignDepthKeyData(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const DepthKey::Data& aNewData)
{
    assignKeyData<DepthKey, TimeKeyType_Depth>(
                aProject, aTarget, aFrame, aNewData,
                aFrame == TimeLine::kDefaultKeyIndex ?
                    CmndName::tr("update a default depth") :
                    CmndName::tr("update a depth key"));
}

void assignOpaKeyData(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const OpaKey::Data& aNewData)
{
    assignKeyData<OpaKey, TimeKeyType_Opa>(
                aProject, aTarget, aFrame, aNewData,
                aFrame == TimeLine::kDefaultKeyIndex ?
                    CmndName::tr("update a default opacity") :
                    CmndName::tr("update a opacity key"));
}

void assignPoseKeyEasing(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const util::Easing::Param& aNewData)
{
    assignKeyEasing<PoseKey, TimeKeyType_Pose>(
                aProject, aTarget, aFrame, aNewData,
                aFrame == TimeLine::kDefaultKeyIndex ?
                    CmndName::tr("update a default posing") :
                    CmndName::tr("update a posing key"));
}

void assignFFDKeyEasing(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        const util::Easing::Param& aNewData)
{
    assignKeyEasing<FFDKey, TimeKeyType_FFD>(
                aProject, aTarget, aFrame, aNewData,
                aFrame == TimeLine::kDefaultKeyIndex ?
                    CmndName::tr("update a default FFD") :
                    CmndName::tr("update a FFD key"));
}

void assignImageKeyResource(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        img::ResourceNode& aNewData)
{
    ResourceUpdatingWorkspacePtr workspace =
            std::make_shared<ResourceUpdatingWorkspace>();
    const bool createTransitions = !aTarget.timeLine()->isEmpty(TimeKeyType_FFD);

    assignKeyBy<ImageKey, TimeKeyType_Image>(
                aProject, aTarget, aFrame,
                aFrame == TimeLine::kDefaultKeyIndex ?
                    CmndName::tr("update a resource of a default image") :
                    CmndName::tr("update a resource of a image key"),
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
                    CmndName::tr("update a center of a default image") :
                    CmndName::tr("update a center of a image key"),
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
                    CmndName::tr("update a mesh cell size of a default image") :
                    CmndName::tr("update a mesh cell size of a image key"),
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
                aProject, aTarget, aFrame, aKey, CmndName::tr("push new moving key"));
}
void pushNewRotateKey(
        Project& aProject, ObjectNode& aTarget, int aFrame, RotateKey* aKey)
{
    pushNewKey<RotateKey, TimeKeyType_Rotate>(
                aProject, aTarget, aFrame, aKey, CmndName::tr("push new rotation key"));
}

void pushNewScaleKey(
        Project& aProject, ObjectNode& aTarget, int aFrame, ScaleKey* aKey)
{
    pushNewKey<ScaleKey, TimeKeyType_Scale>(
                aProject, aTarget, aFrame, aKey, CmndName::tr("push new scaling key"));
}

void pushNewDepthKey(
        Project& aProject, ObjectNode& aTarget, int aFrame, DepthKey* aKey)
{
    pushNewKey<DepthKey, TimeKeyType_Depth>(
                aProject, aTarget, aFrame, aKey, CmndName::tr("push new depth key"));
}

void pushNewOpaKey(
        Project& aProject, ObjectNode& aTarget, int aFrame, OpaKey* aKey)
{
    pushNewKey<OpaKey, TimeKeyType_Opa>(
                aProject, aTarget, aFrame, aKey, CmndName::tr("push new opacity key"));
}

void pushNewPoseKey(
        Project& aProject, ObjectNode& aTarget, int aFrame,
        PoseKey* aKey, BoneKey* aParentKey)
{
    XC_PTR_ASSERT(aParentKey);
    pushNewKey<PoseKey, TimeKeyType_Pose>(
                aProject, aTarget, aFrame, aKey,
                CmndName::tr("push new posing key"), aParentKey);
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
                CmndName::tr("push new FFD key"), aParentKey);
}

void pushNewImageKey(
        Project& aProject, ObjectNode& aTarget, int aFrame, ImageKey* aKey)
{
    pushNewKey<ImageKey, TimeKeyType_Image>(
                aProject, aTarget, aFrame, aKey, CmndName::tr("push new image key"));
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
