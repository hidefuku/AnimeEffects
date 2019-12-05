#ifndef CTRL_TIME_SCALER_H
#define CTRL_TIME_SCALER_H

#include <array>
#include <QPoint>

namespace ctrl {
namespace time {

class Scaler
{
public:
    struct Attribute
    {
        bool showNumber;
        QPoint grid;
    };

    Scaler();

    void setMaxFrame(int aMaxFrame);
    void setFrameList(const std::array<int, 3>& aFrameList);
    void update(int aWheelDelta);
    int pixelWidth(int aFrame) const;
    int maxPixelWidth() const;
    int frame(int aPixelWidth) const;
    int maxFrame() const;
    Attribute attribute(int aFrame) const;

private:
    int mMaxFrame;
    int mWheel;
    int mIndex;
    std::array<int, 3> mFrameList;
};

} // namespace time
} // namespace ctrl

#endif // CTRL_TIME_SCALER_H
