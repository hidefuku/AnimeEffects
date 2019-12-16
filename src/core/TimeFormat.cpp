#include "TimeFormat.h"

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
    case TimeFormatType_Frames_From1:
        return QString::number(aFrame+1);

    case TimeFormatType_Relative_FPS: {
        QString number;
        return number.sprintf("%.1f", static_cast<double>(aFrame) / mFps);
    }

    case TimeFormatType_Seconds_Frames: { // (SS:FF)
        double min = 0.0;
        double max = 60.0;

        double nMin = 0;
        double nMax = 1;

        int rangeMaxSeconds = static_cast<int>(util::MathUtil::remap(mRange.max(),min,max,nMin,nMax));
        double seconds = util::MathUtil::remap(aFrame,min,max,nMin,nMax);

        QString n;
        n.sprintf("%0*d:%02d",
                  QString::number(rangeMaxSeconds).length(), static_cast<int>(seconds),
                  static_cast<int>(util::MathUtil::cycle(aFrame, 0.0, 60.0)));

        return n;
    }

    case TimeFormatType_Timecode_SMPTE: { // (HH:MM:SS:FF)
        double min = 0.0;
        double max = 60.0;

        double nMin = 0;
        double nMax = 1000.0;

        double ms = util::MathUtil::remap(aFrame,min,max,nMin,nMax);

        DDHHMMSSmmm timeStruct = msToDDHHMMSSmmm(ms);

        QString n;
        n.sprintf("%02d:%02d:%02d:%02d",
                  timeStruct.hours, timeStruct.minutes,timeStruct.seconds,
                  static_cast<int>(util::MathUtil::cycle(aFrame, 0.0, 60.0)));

        return n;
    }

    case TimeFormatType_Timecode_HHMMSSmmm: { // (HH:MM:SS:mmm)
        double min = 0.0;
        double max = 60.0;

        double nMin = 0;
        double nMax = 1000.0;

        double ms = util::MathUtil::remap(aFrame,min,max,nMin,nMax);

        DDHHMMSSmmm timeStruct = msToDDHHMMSSmmm(ms);

        QString n;
        n.sprintf("%02d:%02d:%02d:%03d", timeStruct.hours, timeStruct.minutes,timeStruct.seconds,timeStruct.milliseconds);

        return n;
    }

    default:
        return QString::number(aFrame);
    }
}


DDHHMMSSmmm TimeFormat::msToDDHHMMSSmmm(double ms)
{
    int d = 0, h = 0, m = 0, s = 0, mi = 0;
    if(ms >= (86400*1000)) {
        d = qFloor( ((((ms/1000)/60)/60)/24) );
        ms = ( ms-(d*(86400*1000)) );
    }
    if(ms >= (3600*1000)) {
        h = qFloor( ((ms/1000)/60)/60 );
        ms = ( ms-(h*(3600*1000)) );
    }
    if(ms >= (60*1000)) {
        mi = qFloor( (ms/1000)/60 );
        ms = ( ms-(mi*(60*1000)) );
    }
    if(ms >= 1000) {
        s = qFloor(ms/1000);
        ms = ( ms-(s*1000) );
    }
    DDHHMMSSmmm timeStruct;
    timeStruct.days = d;
    timeStruct.hours = h;
    timeStruct.minutes = m;
    timeStruct.seconds = s;
    timeStruct.milliseconds = static_cast<int>(ms);

    return timeStruct;
}

} // namespace core

