#include "thr/Paralleler.h"

namespace thr
{

Paralleler::Paralleler(int aWorkerCount)
    : mQueue()
    , mWorkers()
    , mWorkerCount(aWorkerCount)
{
    for (int i = 0; i < mWorkerCount; ++i)
    {
        mWorkers.emplace_back(std::unique_ptr<Worker>(new Worker(mQueue)));
    }
}

void Paralleler::start(QThread::Priority aPriority)
{
    for (auto& worker : mWorkers)
    {
        worker->start(aPriority);
    }
}

void Paralleler::push(Task& aTask)
{
    mQueue.push(aTask);
}

void Paralleler::cancel(Task& aTask)
{
    mQueue.removeAll(aTask);
    aTask.setCancel();
    aTask.wait();
}

void Paralleler::wakeAll()
{
    mQueue.wakeAll();
}

} // namespace thr
