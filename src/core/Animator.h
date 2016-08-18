#ifndef CORE_ANIMATOR
#define CORE_ANIMATOR

#include "core/Frame.h"

namespace core
{

class Animator
{
public:
    virtual ~Animator() {}
    virtual Frame currentFrame() const = 0;
    virtual void stop() = 0;
    virtual void suspend() = 0;
    virtual void resume() = 0;
    virtual bool isSuspended() const = 0;
};

}

#endif // CORE_ANIMATOR

