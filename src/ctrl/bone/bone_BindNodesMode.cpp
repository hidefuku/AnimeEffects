#include "cmnd/ScopedMacro.h"
#include "core/TimeKeyBlender.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/bone/bone_BindNodesMode.h"
#include "ctrl/bone/bone_Renderer.h"
#include "ctrl/bone/bone_Notifier.h"

using namespace core;

namespace ctrl {
namespace bone {

BindNodesMode::BindNodesMode(Project& aProject, const Target& aTarget,
                             KeyOwner& aKey, const GraphicStyle& aGraphicStyle)
    : mProject(aProject)
    , mGraphicStyle(aGraphicStyle)
    , mTarget(*aTarget.node)
    , mKeyOwner(aKey)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mFocuser()
    , mNodeSelector(*aTarget.node, aGraphicStyle)
{
    XC_PTR_ASSERT(mKeyOwner.key);
    mFocuser.setTopBones(mKeyOwner.key->data().topBones());
    mFocuser.setTargetMatrix(mTargetMtx);
    mFocuser.setFocusConnector(true);


    mNodeSelector.updateGeometry(mProject.currentTimeInfo());
}

bool BindNodesMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    auto focus = mFocuser.update(aCamera, aCursor.screenPos());
    mNodeSelector.updateFocus(aCamera, aCursor.screenPos());

    bool updated = mFocuser.focusChanged() || mNodeSelector.focusChanged();


    if (aCursor.isLeftPressState())
    {
        mNodeSelector.select();

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
    renderer.setFocusConnector(true);

    for (auto bone : mKeyOwner.key->data().topBones())
    {
        renderer.renderBones(bone);
    }

    //renderChildNodes(aInfo, aPainter);
    mNodeSelector.render(aInfo, aPainter);
}

void BindNodesMode::renderChildNodes(const core::RenderInfo& aInfo, QPainter& aPainter)
{
    static const int kRowHeight = 16;
    static const int kMargin = 2;
    static const int kLetterPixelSize = kRowHeight - 2 * kMargin;
    const int kIconWidth = kRowHeight;
    const QSize kIconSize(kIconWidth, kIconWidth);
    const QPoint kIconOffset(-kIconWidth - 2, 0);
    const QColor backColor(0, 0, 0, 200);
    const QColor textColor(255, 255, 255, 255);
    const QBrush textBrush(textColor);
    const QBrush backBrush(backColor);

    QPixmap iconOpen = mGraphicStyle.icon("dooropen").pixmap(kIconSize);

    QFont font = aPainter.font();
    font.setPixelSize(kLetterPixelSize);
    //font.setPointSize(10);
    //font.setFamily("Courier New");
    //font.setFamily("Meiryo");
    font.setStyleStrategy(QFont::StyleStrategy(
                              QFont::PreferOutline |
                              QFont::PreferAntialias |
                              QFont::PreferQuality));
    ///@note In windows, the font antialiasing doesn't work. (seems like a bug?)
    /// The cause is unknown but it's probably avoidable by bold setting.
    font.setBold(true);
    //font.setFixedPitch(false);
    //font.setStretch(QFont::UltraExpanded);
    //font.setLetterSpacing(QFont::PercentageSpacing, 105);
    aPainter.setFont(font);

#if 1
    //aPainter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    //aPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    aPainter.setRenderHint(QPainter::Antialiasing, true);
    aPainter.setRenderHint(QPainter::TextAntialiasing, true);
#endif

    for (auto node : mTarget.children())
    {
        auto nodeName = node->name();

        ///@todo Is this safe to get(calculate if necessary) current position in rendering?
        auto mtx = TimeKeyBlender::getWorldMatrix(*node, mProject.currentTimeInfo());
        auto pos = mtx.column(3).toVector3D();

        aPainter.setPen(Qt::NoPen);
        aPainter.setBrush(backBrush);

        QRect boundingRect = aPainter.fontMetrics().boundingRect(nodeName);

        auto scrPos = aInfo.camera.toScreenPos(pos.toPointF());
        const QRect scrRect(scrPos.toPoint(), boundingRect.size());

        // background
        aPainter.drawRect(scrRect.marginsAdded(QMargins(kMargin, kMargin, kMargin, kMargin)));

        // text
        aPainter.setPen(QPen(textBrush, 1.0f));
        aPainter.drawText(scrRect, Qt::AlignCenter, nodeName);

        // icon
        const QRect iconRect(scrRect.topLeft() + kIconOffset, kIconSize);
        aPainter.drawPixmap(iconRect, iconOpen);
    }
}

} // namespace bone
} // namespace ctrl
