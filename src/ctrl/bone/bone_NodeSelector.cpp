#include "core/TimeKeyBlender.h"
#include "ctrl/bone/bone_NodeSelector.h"

using namespace core;

namespace ctrl {
namespace bone {

NodeSelector::Tag::Tag()
    : node()
    , parent()
    , children()
    , rect()
    , isDir(false)
    , isOpened(false)
{
}

NodeSelector::NodeSelector(ObjectNode& aTopNode, const GraphicStyle& aStyle)
    : mGraphicStyle(aStyle)
    , mTopNode(aTopNode)
    , mTopTag()
    , mCurrentTopTag()
    , mCurrentFocus()
    , mCurrentSelect()
    , mIconFocused(false)
    , mFocusChanged(false)
{
    resetTags(mTopNode, mTopTag);
    mTopTag.isOpened = true;
    mCurrentTopTag = &mTopTag;
}

void NodeSelector::resetTags(ObjectNode& aNode, Tag& aTag)
{
    aTag = Tag();
    aTag.node = &aNode;
    aTag.isDir = aNode.canHoldChild();

    for (auto childNode : aNode.children())
    {
        aTag.children.push_back(Tag());
        resetTags(*childNode, aTag.children.back());
        aTag.children.back().parent = &aTag;
    }
}

void NodeSelector::updateGeometry(const TimeInfo& aTime)
{
    updateGeometryRecursive(mTopTag, aTime);
}

void NodeSelector::updateGeometryRecursive(Tag& aTag, const TimeInfo& aTime)
{
    aTag.rect = getNodeRectF(*aTag.node, aTime);

    for (auto& childTag : aTag.children)
    {
        updateGeometryRecursive(childTag, aTime);
    }
}

ObjectNode* NodeSelector::updateFocus(const CameraInfo& aCamera, const QVector2D& aPos)
{
    const QPointF scrPointF = aPos.toPointF();

    mFocusChanged = false;
    auto prevIconFocused = mIconFocused;
    auto prevCurrentFocus = mCurrentFocus;
    mIconFocused = false;
    mCurrentFocus = nullptr;

    // update intersection with the top
    if (!mCurrentTopTag->invisibleTop())
    {
        if (updateIntersection(*mCurrentTopTag, aCamera, scrPointF))
        {
            mFocusChanged = (prevIconFocused != mIconFocused || prevCurrentFocus != mCurrentFocus);
            return mCurrentTopTag->node;
        }
    }

    // update intersections with children
    for (auto& childTag : mCurrentTopTag->children)
    {
        if (updateIntersection(childTag, aCamera, scrPointF))
        {
            mFocusChanged = (prevIconFocused != mIconFocused || prevCurrentFocus != mCurrentFocus);
            return childTag.node;
        }
    }

    return nullptr;
}

bool NodeSelector::updateIntersection(
        Tag& aTag, const CameraInfo& aCamera, const QPointF& aPos)
{
    XC_ASSERT(!aTag.invisibleTop());

    QRectF scrRect = aTag.rect;
    scrRect.moveTopLeft(aCamera.toScreenPos(scrRect.topLeft()));
    const float iconSize = scrRect.height();

    const QRectF scrIconRect(scrRect.topLeft() + QPointF(-iconSize, 0.0f),
                             QSizeF(iconSize, iconSize));
    if (scrRect.contains(aPos))
    {
        mCurrentFocus = &aTag;
        mIconFocused = false;
        return true;
    }
    else if (scrIconRect.contains(aPos))
    {
        mCurrentFocus = &aTag;
        mIconFocused = true;
        return true;
    }

    return false;
}

QRectF NodeSelector::getNodeRectF(ObjectNode& aNode, const TimeInfo& aInfo) const
{
    auto mtx = TimeKeyBlender::getWorldMatrix(aNode, aInfo);
    auto pos = mtx.column(3).toVector3D();

    const QRect bb = mGraphicStyle.boundingRect(aNode.name());
    return QRectF(pos.toPointF(), QSizeF(bb.size()));
}

void NodeSelector::select()
{
    if (mCurrentFocus && mIconFocused)
    {
        if (mCurrentFocus->isOpened)
        {
            auto parentNode = mCurrentFocus->node->parent();
            XC_PTR_ASSERT(parentNode);
            mCurrentTopTag = mCurrentFocus->parent;
            mCurrentFocus->isOpened = false;
        }
        else
        {
            mCurrentTopTag = mCurrentFocus;
            mCurrentTopTag->isOpened = true;
        }
        mCurrentFocus = nullptr;
        mCurrentSelect = nullptr;
        mIconFocused = false;
    }
    else
    {
        mCurrentSelect = mCurrentFocus;
        mCurrentFocus = nullptr;
        mIconFocused = false;
    }
}

void NodeSelector::clearFocus()
{
    mCurrentFocus = nullptr;
    mIconFocused = false;
    mFocusChanged = true;
}

bool NodeSelector::focusChanged() const
{
    return mFocusChanged;
}

ObjectNode* NodeSelector::selectingNode()
{
    return mCurrentSelect ? mCurrentSelect->node : nullptr;
}

void NodeSelector::clearSelection()
{
    mCurrentSelect = nullptr;
}

void NodeSelector::render(const core::RenderInfo& aInfo, QPainter& aPainter)
{
    auto tagHeight = mCurrentTopTag->rect.height();
    QPixmap iconPix = mGraphicStyle.icon("plus").pixmap(tagHeight);

#if 1
    QFont font = mGraphicStyle.font();
#else
    QFont font = aPainter.font();
    font.setPixelSize(tagHeight);
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
#endif
    aPainter.setFont(font);

#if 1
    //aPainter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    //aPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    aPainter.setRenderHint(QPainter::Antialiasing, true);
    aPainter.setRenderHint(QPainter::TextAntialiasing, true);
#endif

    if (!mCurrentTopTag->invisibleTop())
    {
        renderOneNode(*mCurrentTopTag, iconPix, aInfo, aPainter);
    }

    for (auto childTag : mCurrentTopTag->children)
    {
        renderOneNode(childTag, iconPix, aInfo, aPainter);
    }
}

void NodeSelector::renderOneNode(const Tag& aTag, QPixmap& aIconPix, const RenderInfo& aInfo, QPainter& aPainter)
{
    const QBrush textBrush(QColor(255, 255, 255, 255));
    const QBrush backBrush(QColor(0, 0, 0, 200));

    auto nodeName = aTag.node->name();

    QRectF scrRectF = aTag.rect;
    scrRectF.moveTopLeft(aInfo.camera.toScreenPos(scrRectF.topLeft()));
    const QRect scrRect = scrRectF.toRect();

    aPainter.setPen(Qt::NoPen);
    aPainter.setBrush(backBrush);

    // background
    aPainter.drawRect(scrRect);

    // text
    aPainter.setPen(QPen(textBrush, 1.0f));
    aPainter.drawText(scrRect, Qt::AlignCenter, nodeName);

    // icon
    if (aTag.isDir)
    {
        const int iconWidth = aTag.rect.height();
        const QSize iconSize(iconWidth, iconWidth);
        const QPoint iconOffset(-iconWidth, 0);
        const QRect iconRect(scrRect.topLeft() + iconOffset, iconSize);
        aPainter.drawPixmap(iconRect, aIconPix);
    }
}

} // namespace bone
} // namespace ctrl
