#include "util/MathUtil.h"
#include "core/Constant.h"
#include "ctrl/bone/bone_Renderer.h"

using namespace core;

namespace ctrl {
namespace bone {

Renderer::Renderer(QPainter& aPainter, const RenderInfo& aInfo)
    : mPainter(aPainter)
    , mInfo(aInfo)
    , mTargetMtx()
    , mIsShadow(false)
    , mFocusConnector(false)
{
}

std::array<QPointF, 4> Renderer::getBoneQuad(const QPointF& aFrom, const QPointF& aTo)
{
    std::array<QPointF, 4> quad;

    const QVector2D dir(aTo - aFrom);
    const float length = dir.length();
    const float width = 0.08f * length;
    const QVector2D wing(width * QVector2D(dir.y(), -dir.x()).normalized());
    const QVector2D center(QVector2D(aFrom) + 0.20f * dir);

    quad[0] = aFrom;
    quad[2] = aTo;
    quad[1] = (center + wing).toPointF();
    quad[3] = (center - wing).toPointF();

    return quad;
}

void Renderer::setAntialiasing(bool aFlag)
{
    mPainter.setRenderHint(QPainter::Antialiasing, aFlag);
}

void Renderer::setShadow(bool aIsShadow)
{
    mIsShadow = aIsShadow;
}

void Renderer::setFocusConnector(bool aFocus)
{
    mFocusConnector = aFocus;
}

void Renderer::setTargetMatrix(const QMatrix4x4& aMtx)
{
    mTargetMtx = aMtx;
}

QPointF Renderer::getScreenPointF(const QVector2D& aBonePos) const
{
    return mInfo.camera.toScreenPos(mTargetMtx * QVector3D(aBonePos)).toPointF();
}

void Renderer::renderBones(const Bone2* aBone)
{
    if (!aBone) return;

    const Qt::PenStyle penStyle = mIsShadow ? Qt::DashLine : Qt::SolidLine;
    const QPointF pos = getScreenPointF(aBone->worldPos());


    // each child
    for (const Bone2* child : aBone->children())
    {
        const QColor color = mFocusConnector ?
                    boneColor(child->isSelected(), child->isFocused()) :
                    boneColor(false, false);

        const QBrush brush(color);
        mPainter.setBrush(brush);
        mPainter.setPen(QPen(brush, 1, penStyle));

        const QPointF cpos = getScreenPointF(child->worldPos());
        const float length = QVector2D(cpos - pos).length();

        // draw a bone
        if (length > 0.0f)
        {
            auto quad = getBoneQuad(pos, cpos);
            mPainter.drawLine(quad[0], quad[2]);
            mPainter.drawLine(quad[1], quad[3]);

            mPainter.drawLine(quad[0], quad[1]);
            mPainter.drawLine(quad[1], quad[2]);
            mPainter.drawLine(quad[2], quad[3]);
            mPainter.drawLine(quad[3], quad[0]);
        }

        // draw child
        renderBones(child);
    }

    // draw joint
    renderJoint(*aBone);
}

void Renderer::renderJoint(const Bone2& aBone)
{
    const float range = (aBone.isSelected() || aBone.isFocused()) ? 4.3f : 3.3f;
    const QBrush brush(boneColor(aBone.isSelected(), aBone.isFocused()));
    const QBrush edge(edgeColor(aBone.isSelected(), aBone.isFocused()));
    const QPointF pos = getScreenPointF(aBone.worldPos());

    mPainter.setBrush(brush);
    mPainter.setPen(QPen(QBrush(edge), 1.0f));
    mPainter.drawEllipse(pos, range, range);
}

void Renderer::renderInfluence(const Bone2* aBone)
{
    if (!aBone) return;

    // each child
    for (const Bone2* child : aBone->children())
    {
        //const QColor color = boneColor(false, false);
        //const QColor color(255, 128, 90, 64);
        const QColor color(255, 90, 128, 64);
        const QBrush brush(color);

        mPainter.setBrush(brush);
        mPainter.setPen(QPen(QBrush(QColor(0,0,0,0)), 1));

        // draw
        drawOneInfluence(*aBone, *child);

        // draw child
        renderInfluence(child);
    }
}

void Renderer::renderBrush(const util::Circle& aBrush, bool aPressed)
{
    const QColor idleColor(100, 100, 255, 128);
    const QColor focusColor(255, 255, 255, 255);

    QBrush brush(aPressed ? focusColor : idleColor);
    Qt::PenStyle style = Qt::SolidLine;
    mPainter.setPen(QPen(brush, 1.2f, style));
    mPainter.setBrush(QBrush(Qt::NoBrush));

    auto center = mInfo.camera.toScreenPos(aBrush.center().toPointF());
    auto radius = mInfo.camera.toScreenLength(aBrush.radius());
    mPainter.drawEllipse(center, radius, radius);
}

#if 0
void Renderer::drawFan(const QVector2D& aCenter, const QVector2D& aSwing, float aRadian, float aVScale)
{
    static const int divide = 16;
    const QVector2D h = aSwing.normalized();
    const QVector2D v = util::MathUtil::getRotateVector90Deg(h);

    QPointF p[divide + 2] = { aCenter.toPointF(), (aCenter + aSwing).toPointF() };

    for (int i = 0; i < divide; ++i)
    {
        const float rotate = aRadian * (i + 1) / (float)divide;
        const QVector2D swing = util::MathUtil::getRotateVectorRad(aSwing, rotate);
        const float hdot = QVector2D::dotProduct(h, swing);
        const float vdot = QVector2D::dotProduct(v, swing);
        p[i + 2] = (aCenter + hdot * h + aVScale * vdot * v).toPointF();
    }
    mPainter.drawConvexPolygon(p, divide + 2);
}

void Renderer::drawOneInfluence(const core::Bone2& aParent, const core::Bone2& aChild)
{
    using core::Constant;
    const QVector2D bgn = mInfo.camera.toScreenPos(aParent.worldPos());
    const QVector2D end = mInfo.camera.toScreenPos(aChild.worldPos());
    const QVector2D dir(end - bgn);
    const float len = dir.length();

    if (len > 0.0f)
    {
        const float wing0 = mInfo.camera.scaleByScreen(aChild.range(0).x());
        const float wing1 = mInfo.camera.scaleByScreen(aChild.range(1).x());
        const float yrange0 = mInfo.camera.scaleByScreen(aChild.range(0).y());
        const float yrange1 = mInfo.camera.scaleByScreen(aChild.range(1).y());
        const float vscale0 = xc_divide(yrange0, wing0, Constant::dividable(), 1.0f);
        const float vscale1 = xc_divide(yrange1, wing1, Constant::dividable(), 1.0f);

        const QPointF b = bgn.toPointF();
        const QPointF e = end.toPointF();
        const QPointF h = util::MathUtil::getRotateVector90Deg(dir).normalized().toPointF();
        const QPointF h0 = wing0 * h;
        const QPointF h1 = wing1 * h;
        QPointF p[4] = { b + h0, b - h0, e - h1, e + h1 };
        mPainter.drawConvexPolygon(p, 4);

        drawFan(bgn, QVector2D( h0), (float)M_PI, xc_clamp(vscale0, 0.0f, 1.0f));
        drawFan(end, QVector2D(-h1), (float)M_PI, xc_clamp(vscale1, 0.0f, 1.0f));
    }
}
#else
void Renderer::drawOneInfluence(const core::Bone2& aParent, const core::Bone2& aChild)
{
    if (!aChild.hasValidRange()) return;

    using core::Constant;

    // polygon vertices
    static const int kFanDivide = 16;
    std::array<QPointF, (kFanDivide + 1) * 2> vertices;
    int vtxCount = 0;

    const QVector2D worldPos[2] = { aParent.worldPos(), aChild.worldPos() };
    const QVector2D dir(worldPos[1] - worldPos[0]);
    const float len = dir.length();

    if (len < Constant::normalizable()) return;

    const QVector2D v = dir.normalized();
    const QVector2D h = util::MathUtil::getRotateVector90Deg(v);

    // make fan vertices
    QPointF prevPos;
    for (int t = 0; t < 2; ++t)
    {
        const QVector2D center = worldPos[t];
        const float wing = aChild.range(t).x();
        const float vrange = aChild.range(t).y();
        const float vscale = xc_divide(vrange, wing, Constant::dividable(), 1.0f);
        const QVector2D swing = (t == 0) ? (wing * h) : (-wing * h);

        for (int i = 0; i <= kFanDivide; ++i)
        {
            const float rotate = M_PI * i / (float)kFanDivide;
            const QVector2D offs = util::MathUtil::getRotateVectorRad(swing, rotate);
            const float hdot = QVector2D::dotProduct(h, offs);
            const float vdot = QVector2D::dotProduct(v, offs);
            const QPointF pos = (center + hdot * h + vscale * vdot * v).toPointF();

            if (vtxCount == 0 || prevPos != pos)
            {
                vertices[vtxCount] = pos;
                ++vtxCount;
                prevPos = pos;
            }
        }
    }

    // convert to screen coord
    for (int i = 0; i < vtxCount; ++i)
    {
        vertices[i] = getScreenPointF(QVector2D(vertices[i]));
    }

    // draw
    mPainter.drawPolygon(vertices.data(), vtxCount);
}
#endif

QColor Renderer::boneColor(bool aIsSelected, bool aIsFocused) const
{
    QColor color(120, 120, 200, 235);

    if (aIsSelected)
    {
        color = QColor(200, 130, 255, 255);
    }
    else if (aIsFocused)
    {
        color = QColor(200, 200, 255, 240);
    }

    if (mIsShadow)
    {
        color.setAlpha(75);
        color.setBlue(color.red());
    }

    return color;
}

QColor Renderer::edgeColor(bool aIsSelected, bool aIsFocused) const
{
    if (aIsSelected)
        return QColor(250, 250, 255, 255);
    else if (aIsFocused)
        return QColor(250, 250, 255, 255);
    else
        return QColor(50, 50, 50, 80);
}

} // namespace bone
} // namespace ctrl
