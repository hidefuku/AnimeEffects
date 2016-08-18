#include <QMutexLocker>
#include <QDebug>
#include "thr/Worker.h"

//#define THR_WORKER_DUMP(...) qDebug(__VA_ARGS__)
#define THR_WORKER_DUMP(...)

namespace thr
{

//-------------------------------------------------------------------------------------------------
Worker::Thread::Thread(TaskQueue& aQueue)
    : mQueue(aQueue)
    , mExitLock()
    , mExit(false)
{
}

Worker::Thread::~Thread()
{
    // exit
    {
        QWriteLocker locker(&mExitLock);
        mExit = true;
    }

    // finish remaining tasks
    mQueue.wakeAll();
    wait();

    THR_WORKER_DUMP("destruct worker");
}

void Worker::Thread::run()
{
    while (!isExit())
    {
        Task* task = mQueue.waitPop(1 * 1000);
        if (task)
        {
            task->setRun();
            task->run();
            task->setFinish();
            THR_WORKER_DUMP("worker ran a task");
        }
    }
}

bool Worker::Thread::isExit()
{
    QReadLocker locker(&mExitLock);
    return mExit;
}

//-------------------------------------------------------------------------------------------------
Worker::Worker(TaskQueue& aQueue)
    : mThread(aQueue)
{
}

void Worker::start(QThread::Priority aPriority)
{
    mThread.start(aPriority);
}

} // namespace thr

