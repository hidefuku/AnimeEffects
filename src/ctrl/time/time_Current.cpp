#include "ctrl/time/time_Current.h"

namespace ctrl {
namespace time {

//-------------------------------------------------------------------------------------------------
Current::Current(int aLeftMargin)
    : mLeftMargin(aLeftMargin)
    , mMaxFrame(0)
    , mFrame(0)
    , mPos()
{
    mPos.setY(11);
}

void Current::setMaxFrame(int aMaxFrame)
{
    mMaxFrame = aMaxFrame;
}

void Current::setFrame(const Scaler& aScale, core::Frame aFrame)
{
    mFrame = aFrame;
    mFrame.clamp(0, mMaxFrame);
    mPos.setX(mLeftMargin + aScale.pixelWidth(mFrame.get()));
}

void Current::setHandlePos(const Scaler& aScale, const QPoint& aPos)
{
    mFrame.set(aScale.frame(aPos.x() - mLeftMargin));
    mFrame.clamp(0, mMaxFrame);
    mPos.setX(mLeftMargin + aScale.pixelWidth(mFrame.get()));
}

void Current::update(const Scaler& aScale)
{
    mPos.setX(mLeftMargin + aScale.pixelWidth(mFrame.get()));
}

} // namespace time
} // namespace ctrl
