#ifndef CTRL_MESH_RENDERER_H
#define CTRL_MESH_RENDERER_H

#include <array>
#include <QPainter>
#include <QMatrix4x4>
#include "core/RenderInfo.h"
#include "ctrl/mesh/mesh_MeshAccessor.h"
#include "ctrl/mesh/mesh_Focuser.h"

namespace ctrl {
namespace mesh {

class Renderer
{
public:
    Renderer(QPainter& aPainter, const core::RenderInfo& aInfo);

    void setAntialiasing(bool aFlag);
    void setTargetMatrix(const QMatrix4x4& aMtx);
    void renderMesh(const MeshAccessor& aMesh);
    void renderDangling(const QVector<QVector2D>& aDangling, const QVector2D& aCursor);
    void renderFocus(const Focuser& aFocuser);
    void renderSplitter(const QVector<QVector2D>& aRelay);

private:
    QPointF getScreenPointF(const QVector2D& aShapePos) const;
    void setVtxBrush(bool aFocus) const;
    void setEdgeBrush(bool aFocus) const;
    void setFaceBrush(int aState) const;

    QPainter& mPainter;
    const core::RenderInfo& mInfo;
    QMatrix4x4 mTargetMtx;
};

} // namespace mesh
} // namespace ctrl

#endif // CTRL_MESH_RENDERER_H
