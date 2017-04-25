#include "ctrl/time/time_Scaler.h"

namespace
{
static const int kWheelValue = 120;
static const int kMinScaleRaw =  1 * kWheelValue;
static const int kMaxScaleRaw = 15 * kWheelValue;
}

namespace ctrl {
namespace time {

//-------------------------------------------------------------------------------------------------
Scaler::Scaler()
    : mMaxFrame()
    , mWheel(kWheelValue)
    , mIndex(1)
    , mFrameList()
{
}

void Scaler::setMaxFrame(int aMaxFrame)
{
    mMaxFrame = aMaxFrame;
}

void Scaler::setFrameList(const std::array<int, 3>& aFrameList)
{
    mFrameList = aFrameList;
}

void Scaler::update(int aWheelDelta)
{
#if defined(Q_OS_WIN)
    aWheelDelta = aWheelDelta * -1;
#endif
    mWheel = std::max(kMinScaleRaw, std::min(kMaxScaleRaw, mWheel - aWheelDelta));
    mIndex = mWheel / kWheelValue;
}

int Scaler::pixelWidth(int aFrame) const
{
    const int frame = std::max(0, std::min(mMaxFrame, aFrame));
    return (mIndex + 1) * frame;
}

int Scaler::maxPixelWidth() const
{
    return pixelWidth(mMaxFrame);
}

int Scaler::frame(int aPixelWidth) const
{
    const int frame = (aPixelWidth + ((mIndex + 1) >> 1)) / (mIndex + 1);
    return std::max(0, std::min(mMaxFrame, frame));
}

Scaler::Attribute Scaler::attribute(int aFrame) const
{
    Attribute attr;
    attr.grid.setX((mIndex + 1) * aFrame);

    if (aFrame % mFrameList[0] == 0)
    {
        attr.grid.setY(10);
        attr.showNumber = true;
    }
    else if (aFrame % mFrameList[1] == 0)
    {
        attr.grid.setY(8);
        attr.showNumber = (mIndex >= 3);
    }
    else if (aFrame % mFrameList[2] == 0)
    {
        attr.grid.setY(6);
        attr.showNumber = false;
    }
    else
    {
        attr.grid.setY(3);
        attr.showNumber = false;
    }
    return attr;
}

} // namespace time
} // namespace ctrl
