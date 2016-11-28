#ifndef CTRL_SRT_SYMBOL_H
#define CTRL_SRT_SYMBOL_H

#include <QMatrix4x4>
#include "core/RenderInfo.h"
#include "core/CameraInfo.h"
#include "ctrl/srt/srt_FocusType.h"

namespace ctrl {
namespace srt {

class Symbol
{
public:
    typedef std::pair<FocusType, QVector2D> FocusData;

    Symbol();

    void build(const QMatrix4x4& aLocalMtx,
               const QMatrix4x4& aWorldMtx,
               const core::CameraInfo& aCamera);

    FocusData findFocus(const QVector2D& aWorldPos);

    void draw(const core::RenderInfo& aInfo, QPainter& aPainter, FocusType aFocus);

private:
    QPointF c;
    QPointF p[4];
    QPointF v[4];
    QPointF ev[4];
};

} // namespace srt
} // namespace ctrl

#endif // CTRL_SRT_SYMBOL_H
