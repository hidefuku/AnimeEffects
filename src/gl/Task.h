#ifndef GL_TASK_H
#define GL_TASK_H

#include <QGL>
#include "util/NonCopyable.h"

namespace gl
{

class Task : private util::NonCopyable
{
public:
    Task();
    virtual ~Task();

    void request();
    void finish();

    bool isRunning() const;

private:
    virtual void onRequested() = 0;
    virtual void onFinished() = 0;

    void deleteSync();
    GLsync mSync;
};

} // namespace gl

#endif // GL_TASK_H
