#ifndef CORE_TIMEFORMAT_H
#define CORE_TIMEFORMAT_H

#include <QDebug>

#include <QString>
#include "util/Range.h"

namespace core
{

enum TimeFormatType
{
    TimeFormatType_Frames_From0,   // (FF)
    TimeFormatType_Frames_From1,   // (FF+1)
    TimeFormatType_Relative_FPS,   // (FF / FPS)
    TimeFormatType_Seconds_Frames, // (SS:FF)
    TimeFormatType_Timecode_SMPTE  // (HH:MM:SS:FF)
};

class TimeFormat
{
public:
    TimeFormat(util::Range aRange, int aFps);

    const QString frameToString(const int aFrame, const TimeFormatType aTimeFormatType) const;

private:


    util::Range mRange;
    int mFps;
};

} // namespace core

#endif // CORE_TIMEFORMAT_H
