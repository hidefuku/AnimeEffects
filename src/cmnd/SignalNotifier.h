#ifndef CMND_SIGNALNOTIFIER
#define CMND_SIGNALNOTIFIER

#include "util/Signaler.h"
#include "cmnd/Listener.h"

namespace cmnd
{

class SignalNotifier : public Listener
{
    util::Signaler<void()>& mSignaler;
public:
    SignalNotifier(util::Signaler<void()>& aSignaler)
        : mSignaler(aSignaler)
    {
    }

    virtual void onExecuted()
    {
        mSignaler();
    }

    virtual void onUndone()
    {
        mSignaler();
    }

    virtual void onRedone()
    {
        mSignaler();
    }
};

}

#endif // CMND_SIGNALNOTIFIER

