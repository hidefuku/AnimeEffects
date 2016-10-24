#include "core/TimeKeyBlender.h"
#include "core/Project.h"
#include "ctrl/srt/srt_CentroidMover.h"

namespace ctrl {
namespace srt {

CentroidMover::CentroidMover(core::ObjectNode& aTarget,
                             const QVector2D& aPrev,
                             const QVector2D& aNext)
    : mTarget(aTarget)
    , mPrev(aPrev)
    , mNext(aNext)
    , mKeys()
    , mChildKeys()
    , mImageKeys()
    , mDone()
    , mExecuteOnce()
{
}

void CentroidMover::modifyValue(const QVector2D& aNext)
{
    mNext = aNext;

    if (mExecuteOnce)
    {
        const QVector3D keyMove = QVector3D(mNext - mPrev);
        const QVector2D keyMove2D = keyMove.toVector2D();

        // update srt keys
        for (auto& key : mKeys)
        {
            key.next = key.prev + key.ptr->data().localSRMatrix() * keyMove;
        }

        // update child srt keys
        for (auto& key : mChildKeys)
        {
            key.next = key.prev - keyMove;
        }

        // update image srt keys
        for (auto& key : mImageKeys)
        {
            key.next = key.prev - keyMove2D;
        }

        if (mDone)
        {
            for (auto& key : mKeys)
            {
                key.ptr->data().pos = key.next;
                key.ptr->data().clampPos();
            }
            for (auto& key : mChildKeys)
            {
                key.ptr->data().pos = key.next;
                key.ptr->data().clampPos();
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
    const QVector3D keyMove = QVector3D(mNext - mPrev);
    const QVector2D keyMove2D = keyMove.toVector2D();

    // move srt keys
    {
        auto& map = mTarget.timeLine()->map(core::TimeKeyType_SRT);

        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            auto key = itr.value();
            TIMEKEY_PTR_TYPE_ASSERT(key, SRT);
            core::SRTKey* srtKey = (core::SRTKey*)key;
            const QVector3D prev = srtKey->data().pos;
            const QVector3D next = prev + srtKey->data().localSRMatrix() * keyMove;
            KeyData keyData = { srtKey, prev, next };
            mKeys.push_back(keyData);
        }

        auto defaultKey = mTarget.timeLine()->defaultKey(core::TimeKeyType_SRT);
        if (defaultKey)
        {
            TIMEKEY_PTR_TYPE_ASSERT(defaultKey, SRT);
            core::SRTKey* srtKey = (core::SRTKey*)defaultKey;
            const QVector3D prev = srtKey->data().pos;
            const QVector3D next = prev + srtKey->data().localSRMatrix() * keyMove;
            KeyData keyData = { srtKey, prev, next };
            mKeys.push_back(keyData);
        }
    }

    if (mTarget.canHoldChild())
    {
        // move child srt keys negatively
        for (auto child : mTarget.children())
        {
            auto& map = child->timeLine()->map(core::TimeKeyType_SRT);

            for (auto itr = map.begin(); itr != map.end(); ++itr)
            {
                auto key = itr.value();
                TIMEKEY_PTR_TYPE_ASSERT(key, SRT);
                core::SRTKey* srtKey = (core::SRTKey*)key;
                const QVector3D prev = srtKey->data().pos;
                const QVector3D next = prev - keyMove;
                ChildKeyData keyData = { srtKey, prev, next };
                mChildKeys.push_back(keyData);
            }

            auto defaultKey = child->timeLine()->defaultKey(core::TimeKeyType_SRT);
            if (defaultKey)
            {
                TIMEKEY_PTR_TYPE_ASSERT(defaultKey, SRT);
                core::SRTKey* srtKey = (core::SRTKey*)defaultKey;
                const QVector3D prev = srtKey->data().pos;
                const QVector3D next = prev - keyMove;
                ChildKeyData keyData = { srtKey, prev, next };
                mChildKeys.push_back(keyData);
            }
        }
    }
    else
    {
        // move image keys negatively
        auto& map = mTarget.timeLine()->map(core::TimeKeyType_Image);

        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            auto key = itr.value();
            TIMEKEY_PTR_TYPE_ASSERT(key, Image);
            core::ImageKey* imageKey = (core::ImageKey*)key;
            const QVector2D prev = imageKey->data().imageOffset();
            const QVector2D next = prev - keyMove2D;
            ImageKeyData keyData = { imageKey, prev, next };
            mImageKeys.push_back(keyData);
        }

        auto defaultKey = mTarget.timeLine()->defaultKey(core::TimeKeyType_Image);
        if (defaultKey)
        {
            TIMEKEY_PTR_TYPE_ASSERT(defaultKey, Image);
            core::ImageKey* imageKey = (core::ImageKey*)defaultKey;
            const QVector2D prev = imageKey->data().imageOffset();
            const QVector2D next = prev - keyMove2D;
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
        key.ptr->data().pos = key.prev;
        key.ptr->data().clampPos();
    }
    for (auto& key : mChildKeys)
    {
        key.ptr->data().pos = key.prev;
        key.ptr->data().clampPos();
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
        key.ptr->data().pos = key.next;
        key.ptr->data().clampPos();
    }
    for (auto& key : mChildKeys)
    {
        key.ptr->data().pos = key.next;
        key.ptr->data().clampPos();
    }
    for (auto& key : mImageKeys)
    {
        key.ptr->data().setImageOffset(key.next);
    }
    mDone = true;
}

} // namespace srt
} // namespace ctrl
