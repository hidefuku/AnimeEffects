#ifndef THR_TASKQUEUE_H
#define THR_TASKQUEUE_H

#include <QList>
#include <QMutex>
#include <QWaitCondition>
#include "util/NonCopyable.h"
#include "thr/Task.h"

namespace thr
{

class TaskQueue : private util::NonCopyable
{
public:
    TaskQueue();

    // push a task
    // Ownership of tasks still belong to a caller.
    void push(Task& aTask);

    // remove a task
    void removeAll(Task& aTask);

    // pop a task with proper waiting
    Task* waitPop(unsigned long aMSec);

    // Wake all workers which are waiting for task popping.
    void wakeAll();

private:
    QList<Task*> mTaskList;
    QMutex mLock;
    QWaitCondition mCondition;
    QMutex mCondLock;
};

} // namespace thr

#endif // THR_TASKQUEUE_H
