#include "ctrl/mesh/mesh_Renderer.h"

using namespace core;

namespace ctrl {
namespace mesh {

Renderer::Renderer(QPainter& aPainter, const RenderInfo& aInfo)
    : mPainter(aPainter)
    , mInfo(aInfo)
    , mTargetMtx()
{
}

void Renderer::setAntialiasing(bool aFlag)
{
    mPainter.setRenderHint(QPainter::Antialiasing, aFlag);
}

void Renderer::setTargetMatrix(const QMatrix4x4& aMtx)
{
    mTargetMtx = aMtx;
}

void Renderer::renderMesh(const MeshAccessor& aMesh)
{
    // draw faces
    setFaceBrush(0);
    for (auto& face : aMesh.faces())
    {
        auto vtx = face->vertices();
        const QPointF vpos[3] =
        {
            getScreenPointF(vtx[0]->vec()),
            getScreenPointF(vtx[1]->vec()),
            getScreenPointF(vtx[2]->vec())
        };
        mPainter.drawConvexPolygon(vpos, 3);

        /*
        setEdgeBrush(false);
        mPainter.drawLine(vpos[0], vpos[0] + 50.0f * face->vnorm(0).toPointF());
        mPainter.drawLine(vpos[1], vpos[1] + 50.0f * face->vnorm(1).toPointF());
        mPainter.drawLine(vpos[2], vpos[2] + 50.0f * face->vnorm(2).toPointF());
        setFaceBrush(0);
        */
    }

    // draw edges
    setEdgeBrush(false);
    for (auto& edge : aMesh.edges())
    {
        const QPointF vpos[2] =
        {
            getScreenPointF(edge->vtx(0)->vec()),
            getScreenPointF(edge->vtx(1)->vec())
        };
        mPainter.drawLine(vpos[0], vpos[1]);
    }

    // draw vertices
    setVtxBrush(false);
    for (auto& vtx : aMesh.vertices())
    {
        auto pos = getScreenPointF(vtx->vec());
        mPainter.drawEllipse(pos, 3.5f, 3.5f);
    }
}

void Renderer::renderDangling(const QVector<QVector2D>& aDangling, const QVector2D& aCursor)
{
    auto count = aDangling.size();

    if (count >= 2)
    {
        const QPointF vtx[3] =
        {
            getScreenPointF(aDangling[0]),
            getScreenPointF(aDangling[1]),
            (count >= 3 ?
            getScreenPointF(aDangling[2]) :
            getScreenPointF(aCursor))
        };

        setFaceBrush(1);
        mPainter.drawConvexPolygon(vtx, 3);

        setEdgeBrush(true);
        mPainter.drawLine(vtx[0], vtx[1]);
        mPainter.drawLine(vtx[1], vtx[2]);
        mPainter.drawLine(vtx[2], vtx[0]);
    }
    else if (count >= 1)
    {
        const QPointF vtx[2] =
        {
            getScreenPointF(aDangling[0]),
            getScreenPointF(aCursor)
        };

        setEdgeBrush(true);
        mPainter.drawLine(vtx[0], vtx[1]);
    }

    // draw vertices
    setVtxBrush(true);
    for (auto vtx : aDangling)
    {
        auto pos = getScreenPointF(vtx);
        mPainter.drawEllipse(pos, 3.5f, 3.5f);
    }
}

void Renderer::renderFocus(const Focuser& aFocuser)
{
    // draw faces
    if (aFocuser.faceFocus())
    {
        setFaceBrush(1);
        auto vtx = aFocuser.faceFocus()->vertices();
        const QPointF vpos[3] =
        {
            getScreenPointF(vtx[0]->vec()),
            getScreenPointF(vtx[1]->vec()),
            getScreenPointF(vtx[2]->vec())
        };
        mPainter.drawConvexPolygon(vpos, 3);
    }
    if (aFocuser.selectingFace())
    {
        setFaceBrush(2);
        auto vtx = aFocuser.selectingFace()->vertices();
        const QPointF vpos[3] =
        {
            getScreenPointF(vtx[0]->vec()),
            getScreenPointF(vtx[1]->vec()),
            getScreenPointF(vtx[2]->vec())
        };
        mPainter.drawConvexPolygon(vpos, 3);

    }

    if (aFocuser.edgeFocus() || aFocuser.selectingEdge())
    {
        auto edge = aFocuser.edgeFocus() ?
                    aFocuser.edgeFocus() : aFocuser.selectingEdge();
        const QPointF vpos[2] =
        {
            getScreenPointF(edge->vtx(0)->vec()),
            getScreenPointF(edge->vtx(1)->vec())
        };
        setEdgeBrush(true);
        mPainter.drawLine(vpos[0], vpos[1]);
    }

    if (aFocuser.vtxFocus())
    {
        setVtxBrush(true);
        auto pos = getScreenPointF(aFocuser.vtxFocus()->vec());
        mPainter.drawEllipse(pos, 4.5f, 4.5f);
    }
}

void Renderer::renderSplitter(const QVector<QVector2D>& aRelay)
{
    if (aRelay.size() < 2) return;

    auto color = QColor(255, 100, 230, 255);
    const Qt::PenStyle penStyle = Qt::DashLine;
    mPainter.setBrush(QBrush(Qt::NoBrush));
    mPainter.setPen(QPen(QBrush(color), 1.2f, penStyle));

    // draw segments
    for (int i = 1; i < aRelay.size(); ++i)
    {
        const QPointF v1 = getScreenPointF(aRelay[i - 1]);
        const QPointF v2 = getScreenPointF(aRelay[i]);
        mPainter.drawLine(v1, v2);
    }

    // draw points
    mPainter.setBrush(QBrush(color));
    for (int i = 0; i < aRelay.size(); ++i)
    {
        const QPointF v = getScreenPointF(aRelay[i]);
        mPainter.drawEllipse(v, 3.5f, 3.5f);
    }
}

QPointF Renderer::getScreenPointF(const QVector2D& aShapePos) const
{
    return mInfo.camera.toScreenPos(mTargetMtx * QVector3D(aShapePos)).toPointF();
}

void Renderer::setVtxBrush(bool aFocus) const
{
    auto color1 = aFocus ? QColor(220, 220, 255, 230) : QColor(100, 100, 255, 230);
    auto color2 = aFocus ? QColor(255, 255, 255, 255) : QColor(150, 150, 255, 230);
    const Qt::PenStyle penStyle = Qt::SolidLine;

    mPainter.setBrush(QBrush(color1));
    mPainter.setPen(QPen(QBrush(color2), 1, penStyle));
}

void Renderer::setEdgeBrush(bool aFocus) const
{
    auto color = aFocus ? QColor(220, 220, 255, 255) : QColor(100, 100, 255, 230);
    const Qt::PenStyle penStyle = aFocus ? Qt::SolidLine : Qt::DashLine;

    mPainter.setBrush(QBrush(Qt::NoBrush));
    mPainter.setPen(QPen(QBrush(color), 1.2f, penStyle));
}

void Renderer::setFaceBrush(int aState) const
{
    auto color = aState == 0 ?
                QColor(100, 100, 255, 60) :
                aState == 1 ?
                    QColor(230, 150, 255, 80) :
                    QColor(230, 150, 255, 80);

    mPainter.setBrush(QBrush(color));
    mPainter.setPen(QPen(Qt::NoPen));
}

} // namespace mesh
} // namespace ctrl

