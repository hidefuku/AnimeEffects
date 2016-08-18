#ifndef CORE_TIMEINFO_H
#define CORE_TIMEINFO_H

#include "core/Frame.h"

namespace core
{

class TimeInfo
{
public:
    TimeInfo()
        : fps(0)
        , frameMax(0)
        , loop(false)
        , frame()
    {}

    int fps;
    int frameMax;
    bool loop;
    Frame frame;
};

} // namespace core

#endif // CORE_TIMEINFO_H
