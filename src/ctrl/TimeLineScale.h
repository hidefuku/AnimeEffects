#ifndef CTRL_TIMELINESCALE_H
#define CTRL_TIMELINESCALE_H

#include <array>
#include <QPoint>

namespace ctrl
{

class TimeLineScale
{
public:
    struct Attribute
    {
        bool showNumber;
        QPoint grid;
    };

    TimeLineScale();

    void setMaxFrame(int aMaxFrame);
    void setFrameList(const std::array<int, 3>& aFrameList);
    void update(int aWheelDelta);
    int pixelWidth(int aFrame) const;
    int maxPixelWidth() const;
    int frame(int aPixelWidth) const;
    Attribute attribute(int aFrame) const;

private:
    int mMaxFrame;
    int mWheel;
    int mIndex;
    std::array<int, 3> mFrameList;
};

} // namespace ctrl

#endif // CTRL_TIMELINESCALE_H
