#ifndef CMND_SLEEPABLEOBJECT_H
#define CMND_SLEEPABLEOBJECT_H

namespace cmnd
{

class SleepableObject
{
public:
    virtual ~SleepableObject() {}
    virtual void awake() = 0;
    virtual void sleep() = 0;
};

} // namespace cmnd

#endif // CMND_SLEEPABLEOBJECT_H
