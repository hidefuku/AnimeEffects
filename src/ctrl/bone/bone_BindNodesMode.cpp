#include "cmnd/ScopedMacro.h"
#include "core/TimeKeyBlender.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/bone/bone_BindNodesMode.h"
#include "ctrl/bone/bone_Renderer.h"
#include "ctrl/bone/bone_Notifier.h"

using namespace core;

namespace ctrl {
namespace bone {

BindNodesMode::BindNodesMode(Project& aProject, const Target& aTarget, KeyOwner& aKey)
    : mProject(aProject)
    , mTarget(*aTarget.node)
    , mKeyOwner(aKey)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mFocuser()
{
    XC_PTR_ASSERT(mKeyOwner.key);
    mFocuser.setTopBones(mKeyOwner.key->data().topBones());
    mFocuser.setTargetMatrix(mTargetMtx);
}

bool BindNodesMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    auto focus = mFocuser.update(aCamera, aCursor.screenPos());
    bool updated = mFocuser.focusChanged();

    if (aCursor.isLeftPressState())
    {
        mFocuser.clearFocus();
        mFocuser.clearSelection();
        if (focus)
        {
            mFocuser.select(*focus);
        }
        updated = true;
    }
    else if (aCursor.isLeftMoveState())
    {
    }
    else if (aCursor.isLeftReleaseState())
    {
        mFocuser.clearSelection();
        updated = true;
    }

    return updated;
}

void BindNodesMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
{
    bone::Renderer renderer(aPainter, aInfo);
    renderer.setAntialiasing(true);
    renderer.setTargetMatrix(mTargetMtx);

    for (auto bone : mKeyOwner.key->data().topBones())
    {
        renderer.renderBones(bone);
    }


    QFont font = aPainter.font();
    //font.setPixelSize(16);
    //font.setPixelSize(13);
    font.setPointSize(10);
    //font.setFamily("Courier New");
    //font.setFamily("Meiryo");
    //font.setStyleStrategy(QFont::PreferAntialias);
    font.setStyleStrategy(
                QFont::StyleStrategy(
                    QFont::PreferOutline |
                    QFont::PreferAntialias |
                    QFont::PreferQuality));
    ///@note In windows, the font antialiasing doesn't work. (seems like a bug?)
    /// The cause is unknown but it's probably avoidable by bold setting.
    font.setBold(true);

    //font.setWeight(QFont::Bold);
    //font.setFixedPitch(false);
    //font.setStretch(QFont::UltraExpanded);
    //font.setLetterSpacing(QFont::PercentageSpacing, 120);
    aPainter.setFont(font);

#if 1
    //aPainter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    //aPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    aPainter.setRenderHint(QPainter::Antialiasing, true);
    aPainter.setRenderHint(QPainter::TextAntialiasing, true);
#endif

    for (auto node : mTarget.children())
    {
        ///@todo Is this safe to get(calculate if necessary) current position in rendering?
        auto mtx = TimeKeyBlender::getWorldMatrix(*node, mProject.currentTimeInfo());
        auto pos = mtx.column(3).toVector3D();

        const QColor backColor(0, 0, 0, 200);
        const QColor textColor(255, 255, 255, 255);
        QBrush textBrush(textColor);
        QBrush backBrush(backColor);
        //aPainter.setPen(QPen(backBrush, 2.0f));
        aPainter.setPen(Qt::NoPen);
        aPainter.setBrush(backBrush);

        //const QRect charRectangle = QRect(0, 0, 0, 0);
        //QRect boundingRect;
        //aPainter.drawText(charRectangle, 0, node->name(), &boundingRect);
        QRect boundingRect = aPainter.fontMetrics().boundingRect(node->name());

        auto scrPos = aInfo.camera.toScreenPos(pos.toPointF());
        QRect scrRect(scrPos.toPoint(), boundingRect.size());

        aPainter.drawRect(scrRect.marginsAdded(QMargins(2, 1, 2, 1)));

        //aPainter.setPen(Qt::NoPen);
        aPainter.setPen(QPen(textBrush, 1.0f));
        //aPainter.setBrush(textBrush);
        //aPainter.setBrush(Qt::NoBrush);

        aPainter.drawText(scrRect, Qt::AlignCenter, node->name());
        //aPainter.drawText(scrPos, node->name());
    }
}

} // namespace bone
} // namespace ctrl
