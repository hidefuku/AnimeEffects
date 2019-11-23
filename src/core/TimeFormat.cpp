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
    case TimeFormatType_Frames_From1:    return QString::number(aFrame+1);
    case TimeFormatType_Relative_FPS: {
        QString number;
        return number.sprintf("%.1f", static_cast<double>(aFrame) / mFps);
    }
    case TimeFormatType_Seconds_Frames:  return "";

    case TimeFormatType_Timecode_SMPTE: { // (HH:MM:SS:FF)
        double min = 0.0; //static_cast<double>(mRange.min()) / mFps;
        double max = 60.0; //static_cast<double>(mRange.max()) / mFps;
        double v = static_cast<double>(aFrame); //static_cast<double>(aFrame) / mFps;

        double nMin = 0;
        double nMax = 1000.0;

        double ms = ((((v - min) * (nMax - nMin)) / (max - min)) + nMin);

        QString n;
        n.sprintf("%.3f", v);
        qDebug() << n;

        /*
            function remap(oldValue,oldMin,oldMax,newMin,newMax) {
                if(oldMin === newMin && oldMax === newMax)
                    return oldValue
                // Linear conversion formular
                // NewValue = (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin
                return (((oldValue - oldMin) * (newMax - newMin)) / (oldMax - oldMin)) + newMin;
            }
        */

        /*
        function to24HClockObject(ms) {

            var d = 0
            var h = 0
            var mi = 0
            var s = 0

            if(ms >= (86400*1000)) {
                d = Math.floor((((ms/1000)/60)/60)/24)
                ms = (ms-(d*(86400*1000)))
            }
            if(ms >= (3600*1000)) {
                h = Math.floor(((ms/1000)/60)/60)
                ms = (ms-(h*(3600*1000)))
            }
            if(ms >= (60*1000)) {
                mi = Math.floor((ms/1000)/60)
                ms = (ms-(mi*(60*1000)))
            }
            if(ms >= 1000) {
                s = Math.floor(ms/1000)
                ms = (ms-(s*1000))
            }

            var arr = {}
            arr.days = pad(d,2)
            arr.hours = pad(h,2)
            arr.minutes = pad(mi,2)
            arr.seconds = pad(s,2)
            arr.milliseconds = pad(ms,3,0)
            return arr
        }
        */

        return n;
    }
    default:                             return QString::number(aFrame);
    }
}

} // namespace core

