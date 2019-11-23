#ifndef CORE_TIMEFORMAT_H
#define CORE_TIMEFORMAT_H

#include <QString>
#include "util/Range.h"

namespace core
{

enum TimeFormatType
{
    TimeFormatType_Frames_From0,
    TimeFormatType_Frames_From1,
    TimeFormatType_Relative_FPS,
    TimeFormatType_Seconds_Frames,
    TimeFormatType_Timecode_SMPTE
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
