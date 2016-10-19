#include "core/TimeKeyBlender.h"
#include "core/Project.h"
#include "ctrl/srt/srt_CentroidMover.h"

namespace ctrl {
namespace srt {

CentroidMover::CentroidMover(
        core::ObjectNode& aTarget,
        const QVector2D& aNext)
    : mTarget(aTarget)
    , mPrev()
    , mNext(aNext)
    , mKeys()
    , mChildKeys()
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

        if (mDone)
        {
            mTarget.setInitialCenter(mNext);

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
        }
    }
}

void CentroidMover::exec()
{
    mPrev = mTarget.initialCenter();
    const QVector3D keyMove = QVector3D(mNext - mPrev);

    // move srt keys
    {
        auto& map = mTarget.timeLine()->map(core::TimeKeyType_SRT);

        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            core::TimeKey* key = itr.value();
            TIMEKEY_PTR_TYPE_ASSERT(key, SRT);

            core::SRTKey* srtKey = (core::SRTKey*)key;
            const QVector3D prev = srtKey->data().pos;
            const QVector3D next = prev + srtKey->data().localSRMatrix() * keyMove;
            KeyData keyData = { srtKey, prev, next };
            mKeys.push_back(keyData);
        }
    }

    // move child srt keys negatively
    for (auto child : mTarget.children())
    {
        auto& map = child->timeLine()->map(core::TimeKeyType_SRT);

        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            core::TimeKey* key = itr.value();
            TIMEKEY_PTR_TYPE_ASSERT(key, SRT);
            core::SRTKey* srtKey = (core::SRTKey*)key;

            const QVector3D prev = srtKey->data().pos;
            const QVector3D next = prev - keyMove;
            KeyData keyData = { srtKey, prev, next };
            mChildKeys.push_back(keyData);
        }
    }

    redo();

    mExecuteOnce = true;
}

void CentroidMover::undo()
{
    mTarget.setInitialCenter(mPrev);

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
    mDone = false;
}

void CentroidMover::redo()
{
    mTarget.setInitialCenter(mNext);

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
    mDone = true;
}

} // namespace srt
} // namespace ctrl
