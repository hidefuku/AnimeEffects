#ifndef CTRL_BONE_NODESELECTOR_H
#define CTRL_BONE_NODESELECTOR_H

#include "core/ObjectNode.h"
#include "core/CameraInfo.h"
#include "core/TimeInfo.h"
#include "ctrl/GraphicStyle.h"

namespace ctrl {
namespace bone {

class NodeSelector
{
public:
    NodeSelector(core::ObjectNode& aTopNode, const GraphicStyle& aStyle);

    void updateGeometry(const core::TimeInfo& aTime);
    core::ObjectNode* updateFocus(const core::CameraInfo& aCamera, const QVector2D& aPos);

    void select();

    void clearFocus();
    bool focusChanged() const;

    core::ObjectNode* selectingNode();
    void clearSelection();

    void render(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    class Tag
    {
    public:
        Tag();
        core::ObjectNode* node;
        Tag* parent;
        QList<Tag> children;
        QRectF rect;
        bool isDir;
        bool isOpened;
        bool invisibleTop() const { return !parent; }
    };

    void resetTags(core::ObjectNode& aNode, Tag& aTag);
    void updateGeometryRecursive(Tag& aTag, const core::TimeInfo& aTime);
    QRectF getNodeRectF(core::ObjectNode& aNode, const core::TimeInfo& aInfo) const;
    bool updateIntersection(Tag& aTag, const core::CameraInfo& aCamera, const QPointF& aPos);
    void renderOneNode(const Tag& aTag, QPixmap& aIconPix, const core::RenderInfo& aInfo, QPainter& aPainter);

    const GraphicStyle& mGraphicStyle;
    core::ObjectNode& mTopNode;
    Tag mTopTag;
    Tag* mCurrentTopTag;
    Tag* mCurrentFocus;
    Tag* mCurrentSelect;
    bool mIconFocused;
    bool mFocusChanged;
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_NODESELECTOR_H
