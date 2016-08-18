#include <QMutexLocker>
#include "thr/TaskQueue.h"

namespace thr
{

TaskQueue::TaskQueue()
    : mTaskList()
    , mLock()
    , mCondition()
    , mCondLock()
{
}

void TaskQueue::push(Task& aTask)
{
    QMutexLocker locker(&mLock);
    aTask.setIdle();
    mTaskList.push_back(&aTask);
}

void TaskQueue::removeAll(Task& aTask)
{
    QMutexLocker locker(&mLock);
    mTaskList.removeAll(&aTask);
}

Task* TaskQueue::waitPop(unsigned long aMSec)
{
    mLock.lock();

    // wait if empty
    if (mTaskList.empty())
    {
        mLock.unlock();

        // sleep
        QMutexLocker condLocker(&mCondLock);
        mCondition.wait(&mCondLock, aMSec);
        return nullptr;
    }

    // pop task
    Task* task = mTaskList.front();
    mTaskList.pop_front();

    mLock.unlock();
    return task;
}

void TaskQueue::wakeAll()
{
    QMutexLocker condLocker(&mCondLock);
    mCondition.wakeAll();
}

} // namespace thr
