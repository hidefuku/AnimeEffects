#ifndef CTRL_TIME_RENDERER_H
#define CTRL_TIME_RENDERER_H

#include <QVector>
#include <QPainter>
#include "util/Range.h"
#include "core/CameraInfo.h"
#include "core/ObjectNode.h"
#include "ctrl/TimeLineRow.h"
#include "ctrl/time/time_Scaler.h"

namespace ctrl {
namespace time {

class Renderer
{
public:
    Renderer(QPainter& aPainter, const core::CameraInfo& aCamera);

    void setMargin(int aMargin) { mMargin = aMargin; }
    void setRange(const util::Range& aRange) { mRange = aRange; }
    void setTimeScale(const Scaler& aScale) { mScale = &aScale; }

    void renderLines(const QVector<TimeLineRow>& aRows, const QRect& aCameraRect, const QRect& aCullRect);
    void renderHeader(int aHeight, int aFps);
    void renderHandle(const QPoint& aPoint, int aRange);
    void renderSelectionRange(const QRect& aRect);

private:
    void drawTimeCurrent();
    void drawKeys(const core::ObjectNode* aNode, const TimeLineRow& aRow);
    void drawChildKeys(const core::ObjectNode* aNode, const QPoint& aPos);

    QPainter& mPainter;
    const core::CameraInfo& mCamera;
    int mMargin;
    util::Range mRange;
    const Scaler* mScale;
};

} // namespace time
} // namespace ctrl

#endif // CTRL_TIME_RENDERER_H
