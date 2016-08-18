#ifndef CTRL_BONE_RENDERER_H
#define CTRL_BONE_RENDERER_H

#include <array>
#include <QPainter>
#include <QMatrix4x4>
#include "util/Circle.h"
#include "core/RenderInfo.h"
#include "core/Bone2.h"

namespace ctrl {
namespace bone {

class Renderer
{
public:
    Renderer(QPainter& aPainter, const core::RenderInfo& aInfo);

    void setAntialiasing(bool aFlag);
    void setShadow(bool aIsShadow);
    void setTargetMatrix(const QMatrix4x4& aMtx);
    void setFocusConnector(bool aFocus);
    void renderBones(const core::Bone2* aTopBone);
    void renderJoint(const core::Bone2& aBone);
    void renderInfluence(const core::Bone2* aTopBone);
    void renderBrush(const util::Circle& aBrush, bool aPressed);

    static std::array<QPointF, 4> getBoneQuad(const QPointF& aFrom, const QPointF& aTo);

private:
    //void drawFan(const QVector2D& aCenter, const QVector2D& aSwing, float aRadian, float aVScale);
    void drawOneInfluence(const core::Bone2& aParent, const core::Bone2& aChild);
    QColor boneColor(bool aIsSelected, bool aIsFocused) const;
    QColor edgeColor(bool aIsSelected, bool aIsFocused) const;
    QPointF getScreenPointF(const QVector2D& aBonePos) const;

    QPainter& mPainter;
    const core::RenderInfo& mInfo;
    QMatrix4x4 mTargetMtx;
    bool mIsShadow;
    bool mFocusConnector;
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_RENDERER_H
