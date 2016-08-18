#ifndef CORE_TIMECACHELOCK_H
#define CORE_TIMECACHELOCK_H

#include <QReadWriteLock>

namespace core
{

class TimeCacheLock
{
public:
    TimeCacheLock();
    QReadWriteLock working;
    QReadWriteLock current;
};

} // namespace core

#endif // CORE_TIMECACHELOCK_H
