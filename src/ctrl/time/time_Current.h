#ifndef CTRL_TIME_CURRENT_H
#define CTRL_TIME_CURRENT_H

#include <QPoint>
#include "core/Frame.h"
#include "ctrl/time/time_Scaler.h"

namespace ctrl {
namespace time {

class Current
{
public:
    Current(int aLeftMargin);
    void setMaxFrame(int aMaxFrame);
    void setFrame(const Scaler& aScale, core::Frame aFrame);
    void setHandlePos(const Scaler& aScale, const QPoint& aPos);
    void update(const Scaler& aScale);
    core::Frame frame() const { return mFrame; }
    const QPoint& handlePos() const { return mPos; }
    int handleRange() const { return 5; }
private:
    const int mLeftMargin;
    int mMaxFrame;
    core::Frame mFrame;
    QPoint mPos;
};

} // namespace time
} // namespace ctrl

#endif // CTRL_TIME_CURRENT_H
