#ifndef THR_TASK
#define THR_TASK

#include <QReadWriteLock>
namespace thr { class Paralleler; }
namespace thr { class Worker; }
namespace thr { class TaskQueue; }

namespace thr
{

class Task
{
    friend class Paralleler;
    friend class Worker;
    friend class TaskQueue;
public:
    Task();
    virtual ~Task();

    void wait() const;
    bool isFinished() const;
    bool isRunning() const;
    bool isCanceling() const;

protected:
    virtual void run() = 0;

private:
    enum State
    {
        State_Idle,
        State_Run,
        State_Finish
    };

    void setIdle();
    void setRun();
    void setFinish();
    void setCancel();

    State mState;
    bool mIsCanceling;
    mutable QReadWriteLock mLock;
};

} // namespace thr

#endif // THR_TASK

