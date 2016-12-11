#include "core/TimeKeyBlender.h"
#include "core/Project.h"
#include "core/TimeKeyBlender.h"
#include "core/MoveKey.h"
#include "core/ImageKey.h"
#include "core/MeshKey.h"
#include "ctrl/srt/srt_CentroidMover.h"

using namespace core;

namespace ctrl {
namespace srt {

//-------------------------------------------------------------------------------------------------
void addAllKeysToEvent(ObjectNode& aTarget, TimeKeyType aType, TimeLineEvent& aEvent, bool aContainDefault)
{
    XC_PTR_ASSERT(aTarget.timeLine());
    auto& line = *aTarget.timeLine();
    {
        auto& map = line.map(aType);
        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            aEvent.pushTarget(aTarget, aType, itr.key());
        }
    }

    if (aContainDefault && line.defaultKey(aType))
    {
        aEvent.pushDefaultTarget(aTarget, aType);
    }

}

//-------------------------------------------------------------------------------------------------
void CentroidMover::pushEventTargets(
        ObjectNode& aTarget, TimeLineEvent& aEvent, bool aAdjustPostures)
{
    if (aAdjustPostures)
    {
        addAllKeysToEvent(aTarget, TimeKeyType_Move, aEvent, true);
    }
    addAllKeysToEvent(aTarget, TimeKeyType_Mesh, aEvent, true);
    addAllKeysToEvent(aTarget, TimeKeyType_Image, aEvent, true);

    for (auto child : aTarget.children())
    {
        addAllKeysToEvent(*child, TimeKeyType_Move, aEvent, true);
    }
}

//-------------------------------------------------------------------------------------------------
CentroidMover::CentroidMover(Project& aProject,
                             ObjectNode& aTarget,
                             const QVector2D& aPrev,
                             const QVector2D& aNext,
                             bool aAdjustPostures)
    : mProject(aProject)
    , mTarget(aTarget)
    , mPrev(aPrev)
    , mNext(aNext)
    , mAdjustsPostures(aAdjustPostures)
    , mKeys()
    , mChildKeys()
    , mImageKeys()
    , mMeshKeys()
    , mDone()
    , mExecuteOnce()
{
}

QMatrix4x4 CentroidMover::getLocalSRMatrix(const TimeKey& aKey)
{
    if (aKey.frame() == TimeLine::kDefaultKeyIndex)
    {
        return QMatrix4x4();
    }
    auto time = mProject.currentTimeInfo();
    time.frame.set(aKey.frame());
    return TimeKeyBlender::getLocalSRMatrix(mTarget, time);
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

        // update meshKeys
        for (auto& key : mMeshKeys)
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
            for (auto& key : mMeshKeys)
            {
                key.ptr->data().setOriginOffset(key.next);
            }
        }
    }
}

void CentroidMover::addAllTargets(
        ObjectNode& aTarget, TimeKeyType aType, bool aContainDefault,
        const std::function<void(TimeKey*)>& aPusher)
{
    XC_PTR_ASSERT(aTarget.timeLine());

    auto& map = aTarget.timeLine()->map(aType);
    for (auto itr = map.begin(); itr != map.end(); ++itr)
    {
        auto key = itr.value();
        aPusher(key);
    }

    if (aContainDefault)
    {
        auto defaultKey = aTarget.timeLine()->defaultKey(aType);
        if (defaultKey)
        {
            aPusher(defaultKey);
        }
    }
}

void CentroidMover::exec()
{
    const QVector2D keyMove = mNext - mPrev;
    const QVector3D keyMove3D(keyMove);

    if (mAdjustsPostures)
    {
        // translate moveKeys
        addAllTargets(mTarget, TimeKeyType_Move, true, [=](TimeKey* aKey)
        {
            TIMEKEY_PTR_TYPE_ASSERT(aKey, Move);
            MoveKey* moveKey = (MoveKey*)aKey;
            const QVector2D prev = moveKey->pos();
            const QVector2D next = prev + (getLocalSRMatrix(*moveKey) * keyMove3D).toVector2D();
            KeyData keyData = { moveKey, prev, next };
            this->mKeys.push_back(keyData);
        });
    }

    // translate imageKeys negatively
    addAllTargets(mTarget, TimeKeyType_Image, true, [=](TimeKey* aKey)
    {
        TIMEKEY_PTR_TYPE_ASSERT(aKey, Image);
        ImageKey* imageKey = (ImageKey*)aKey;
        const QVector2D prev = imageKey->data().imageOffset();
        const QVector2D next = prev - keyMove;
        ImageKeyData keyData = { imageKey, prev, next };
        this->mImageKeys.push_back(keyData);
    });

    // translate meshKeys negatively
    addAllTargets(mTarget, TimeKeyType_Mesh, true, [=](TimeKey* aKey)
    {
        TIMEKEY_PTR_TYPE_ASSERT(aKey, Mesh);
        MeshKey* meshKey = (MeshKey*)aKey;
        const QVector2D prev = meshKey->data().originOffset();
        const QVector2D next = prev - keyMove;
        MeshKeyData keyData = { meshKey, prev, next };
        this->mMeshKeys.push_back(keyData);
    });

    if (mTarget.canHoldChild())
    {
        // translate child moveKeys negatively
        for (auto child : mTarget.children())
        {
            addAllTargets(*child, TimeKeyType_Move, true, [=](TimeKey* aKey)
            {
                TIMEKEY_PTR_TYPE_ASSERT(aKey, Move);
                MoveKey* moveKey = (MoveKey*)aKey;
                const QVector2D prev = moveKey->pos();
                const QVector2D next = prev - keyMove;
                ChildKeyData keyData = { moveKey, prev, next };
                this->mChildKeys.push_back(keyData);
            });
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
    for (auto& key : mMeshKeys)
    {
        key.ptr->data().setOriginOffset(key.prev);
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
    for (auto& key : mMeshKeys)
    {
        key.ptr->data().setOriginOffset(key.next);
    }
    mDone = true;
}

} // namespace srt
} // namespace ctrl
