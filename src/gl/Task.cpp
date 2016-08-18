#include "XC.h"
#include "gl/Task.h"
#include "gl/Util.h"
#include "gl/ExtendShader.h"
#include "gl/Global.h"

namespace gl
{

Task::Task()
    : mSync(0)
{
}

Task::~Task()
{
    deleteSync();
}

void Task::request()
{
    deleteSync();

    onRequested();

    Global::Functions& ggl = Global::functions();
    mSync = ggl.glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    ggl.glFlush();
    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
}

bool Task::isRunning() const
{
    if (mSync == 0) return false;

    GLenum result = Global::functions().glClientWaitSync(mSync, 0, 0);
    XC_ASSERT(result != GL_INVALID_VALUE);
    XC_ASSERT(result != GL_WAIT_FAILED);

    return !(result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED);
}

void Task::finish()
{
    static const GLuint64 kTimeOutNanoSec = 1000 * 1000;
    Global::Functions& ggl = Global::functions();

    while (mSync)
    {
        GLenum result = ggl.glClientWaitSync(mSync, 0, kTimeOutNanoSec);
        XC_ASSERT(result != GL_INVALID_VALUE);
        XC_ASSERT(result != GL_WAIT_FAILED);

        if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED)
        {
            deleteSync();
            break;
        }
    }
    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);

    onFinished();
}

void Task::deleteSync()
{
    if (mSync)
    {
        Global::functions().glDeleteSync(mSync);
        mSync = 0;
    }
}

} // namespace gl
