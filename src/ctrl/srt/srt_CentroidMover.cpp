#include "core/TimeKeyBlender.h"
#include "core/Project.h"
#include "core/TimeKeyBlender.h"
#include "ctrl/srt/srt_CentroidMover.h"

using namespace core;

namespace ctrl {
namespace srt {

//-------------------------------------------------------------------------------------------------
void CentroidMover::pushEventTargets(ObjectNode& aTarget, TimeLineEvent& aEvent)
{
    XC_PTR_ASSERT(aTarget.timeLine());
    {
        auto& map = aTarget.timeLine()->map(TimeKeyType_Move);
        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            aEvent.pushTarget(aTarget, TimeKeyType_Move, itr.key());
        }
    }

    if (aTarget.timeLine()->defaultKey(TimeKeyType_Move))
    {
        aEvent.pushDefaultTarget(aTarget, TimeKeyType_Move);
    }

    if (aTarget.canHoldChild())
    {
        for (auto child : aTarget.children())
        {
            XC_PTR_ASSERT(child->timeLine());
            auto& map = child->timeLine()->map(TimeKeyType_Move);
            for (auto itr = map.begin(); itr != map.end(); ++itr)
            {
                aEvent.pushTarget(*child, TimeKeyType_Move, itr.key());
            }
        }
    }
    else
    {
        auto& map = aTarget.timeLine()->map(TimeKeyType_Image);
        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            aEvent.pushTarget(aTarget, TimeKeyType_Image, itr.key());
        }
    }
}

//-------------------------------------------------------------------------------------------------
CentroidMover::CentroidMover(core::Project& aProject,
                             core::ObjectNode& aTarget,
                             const QVector2D& aPrev,
                             const QVector2D& aNext)
    : mProject(aProject)
    , mTarget(aTarget)
    , mPrev(aPrev)
    , mNext(aNext)
    , mKeys()
    , mChildKeys()
    , mImageKeys()
    , mDone()
    , mExecuteOnce()
{
}

QMatrix4x4 CentroidMover::getLocalSRMatrix(const core::TimeKey& aKey)
{
    if (aKey.frame() == TimeLine::kDefaultKeyIndex)
    {
        return QMatrix4x4();
    }
    auto time = mProject.currentTimeInfo();
    time.frame.set(aKey.frame());
    return core::TimeKeyBlender::getLocalSRMatrix(mTarget, time);
}

void CentroidMover::modifyValue(const QVector2D& aNext)
{
    mNext = aNext;

    if (mExecuteOnce)
    {
        const QVector2D keyMove = mNext - mPrev;
        const QVector3D keyMove3D(keyMove);

        // update moveKeys
        for (auto& key : mKeys)
        {
            key.next = key.prev + (getLocalSRMatrix(*key.ptr) * keyMove3D).toVector2D();
        }

        // update child moveKeys
        for (auto& key : mChildKeys)
        {
            key.next = key.prev - keyMove;
        }

        // update imageKeys
        for (auto& key : mImageKeys)
        {
            key.next = key.prev - keyMove;
        }

        if (mDone)
        {
            for (auto& key : mKeys)
            {
                key.ptr->setPos(key.next);
            }
            for (auto& key : mChildKeys)
            {
                key.ptr->setPos(key.next);
            }
            for (auto& key : mImageKeys)
            {
                key.ptr->data().setImageOffset(key.next);
            }
        }
    }
}

void CentroidMover::exec()
{
    const QVector2D keyMove = mNext - mPrev;
    const QVector3D keyMove3D(keyMove);

    // translate moveKeys
    {
        auto& map = mTarget.timeLine()->map(core::TimeKeyType_Move);

        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            auto key = itr.value();
            TIMEKEY_PTR_TYPE_ASSERT(key, Move);
            core::MoveKey* moveKey = (core::MoveKey*)key;
            const QVector2D prev = moveKey->pos();
            const QVector2D next = prev + (getLocalSRMatrix(*moveKey) * keyMove3D).toVector2D();
            KeyData keyData = { moveKey, prev, next };
            mKeys.push_back(keyData);
        }

        auto defaultKey = mTarget.timeLine()->defaultKey(core::TimeKeyType_Move);
        if (defaultKey)
        {
            TIMEKEY_PTR_TYPE_ASSERT(defaultKey, Move);
            core::MoveKey* moveKey = (core::MoveKey*)defaultKey;
            const QVector2D prev = moveKey->pos();
            const QVector2D next = prev + (getLocalSRMatrix(*moveKey) * keyMove3D).toVector2D();
            KeyData keyData = { moveKey, prev, next };
            mKeys.push_back(keyData);
        }
    }

    if (mTarget.canHoldChild())
    {
        // translate child moveKeys negatively
        for (auto child : mTarget.children())
        {
            auto& map = child->timeLine()->map(core::TimeKeyType_Move);

            for (auto itr = map.begin(); itr != map.end(); ++itr)
            {
                auto key = itr.value();
                TIMEKEY_PTR_TYPE_ASSERT(key, Move);
                core::MoveKey* moveKey = (core::MoveKey*)key;
                const QVector2D prev = moveKey->pos();
                const QVector2D next = prev - keyMove;
                ChildKeyData keyData = { moveKey, prev, next };
                mChildKeys.push_back(keyData);
            }

            auto defaultKey = child->timeLine()->defaultKey(core::TimeKeyType_Move);
            if (defaultKey)
            {
                TIMEKEY_PTR_TYPE_ASSERT(defaultKey, Move);
                core::MoveKey* moveKey = (core::MoveKey*)defaultKey;
                const QVector2D prev = moveKey->pos();
                const QVector2D next = prev - keyMove;
                ChildKeyData keyData = { moveKey, prev, next };
                mChildKeys.push_back(keyData);
            }
        }
    }
    else
    {
        // translate imageKeys negatively
        auto& map = mTarget.timeLine()->map(core::TimeKeyType_Image);

        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            auto key = itr.value();
            TIMEKEY_PTR_TYPE_ASSERT(key, Image);
            core::ImageKey* imageKey = (core::ImageKey*)key;
            const QVector2D prev = imageKey->data().imageOffset();
            const QVector2D next = prev - keyMove;
            ImageKeyData keyData = { imageKey, prev, next };
            mImageKeys.push_back(keyData);
        }

        auto defaultKey = mTarget.timeLine()->defaultKey(core::TimeKeyType_Image);
        if (defaultKey)
        {
            TIMEKEY_PTR_TYPE_ASSERT(defaultKey, Image);
            core::ImageKey* imageKey = (core::ImageKey*)defaultKey;
            const QVector2D prev = imageKey->data().imageOffset();
            const QVector2D next = prev - keyMove;
            ImageKeyData keyData = { imageKey, prev, next };
            mImageKeys.push_back(keyData);
        }
    }

    redo();

    mExecuteOnce = true;
}

void CentroidMover::undo()
{
    for (auto& key : mKeys)
    {
        key.ptr->setPos(key.prev);
    }
    for (auto& key : mChildKeys)
    {
        key.ptr->setPos(key.prev);
    }
    for (auto& key : mImageKeys)
    {
        key.ptr->data().setImageOffset(key.prev);
    }
    mDone = false;
}

void CentroidMover::redo()
{
    for (auto& key : mKeys)
    {
        key.ptr->setPos(key.next);
    }
    for (auto& key : mChildKeys)
    {
        key.ptr->setPos(key.next);
    }
    for (auto& key : mImageKeys)
    {
        key.ptr->data().setImageOffset(key.next);
    }
    mDone = true;
}

} // namespace srt
} // namespace ctrl
