#include "TimeFormat.h"
#include "XC.h"

namespace core
{

TimeFormat::TimeFormat(util::Range aRange, int aFps)
{
    mRange = aRange;
    mFps = aFps;
}

const QString TimeFormat::frameToString(const int aFrame, const TimeFormatType aTimeFormatType) const
{
    switch (aTimeFormatType)
    {
    case TimeFormatType_Frames_From1:    return QString::number(aFrame+1);
    case TimeFormatType_Relative_FPS: {
        QString number;
        return number.sprintf("%.1f", static_cast<double>(aFrame) / mFps);
    }
    case TimeFormatType_Seconds_Frames:  return "";
    case TimeFormatType_Timecode_SMPTE:  return "";
    default:                             return QString::number(aFrame);
    }
}

} // namespace core

