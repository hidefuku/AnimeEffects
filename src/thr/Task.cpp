#include <QThread>
#include "thr/Task.h"

namespace thr
{

Task::Task()
    : mState(State_Idle)
    , mIsCanceling(false)
    , mLock()
{
}

Task::~Task()
{
    wait();
}

void Task::wait() const
{
    while (isRunning())
    {
        QThread::msleep(1);
    }
}

bool Task::isFinished() const
{
    QReadLocker locker(&mLock);
    return mState == State_Finish;
}

bool Task::isRunning() const
{
    QReadLocker locker(&mLock);
    return mState == State_Run;
}

bool Task::isCanceling() const
{
    QReadLocker locker(&mLock);
    return mIsCanceling;
}

void Task::setIdle()
{
    QWriteLocker locker(&mLock);
    mState = State_Idle;
}

void Task::setRun()
{
    QWriteLocker locker(&mLock);
    mState = State_Run;
}

void Task::setFinish()
{
    QWriteLocker locker(&mLock);
    mState = State_Finish;
    mIsCanceling = false;
}

void Task::setCancel()
{
    QWriteLocker locker(&mLock);
    if (mState == State_Run)
    {
        mIsCanceling = true;
    }
}

} // namespace thr
