#ifndef CMND_LISTENER_H
#define CMND_LISTENER_H

namespace cmnd
{

class Listener
{
public:
    virtual ~Listener() {}
    virtual void onExecuted() {}
    virtual void onUndone() {}
    virtual void onRedone() {}
};

} // namespace cmnd

#endif // LISTENER_H
