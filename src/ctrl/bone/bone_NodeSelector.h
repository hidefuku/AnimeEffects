#ifndef CTRL_BONE_NODESELECTOR_H
#define CTRL_BONE_NODESELECTOR_H

#include <vector>
#include "core/ObjectNode.h"
#include "core/CameraInfo.h"
#include "ctrl/GraphicStyle.h"

namespace ctrl {
namespace bone {

class NodeSelector
{
public:
    NodeSelector(core::ObjectNode& aTopNode, const GraphicStyle& aStyle);

    void initGeometries();
    void sortCurrentGeometries(const core::CameraInfo& aCamera);
    core::ObjectNode* updateFocus(const core::CameraInfo& aCamera, const QVector2D& aPos);

    core::ObjectNode* click(const core::CameraInfo& aCamera);

    void clearFocus();
    bool focusChanged() const;

    core::ObjectNode* selectingNode();
    void clearSelection();

    void renderBindings(const core::RenderInfo& aInfo, QPainter& aPainter,
                        const QMatrix4x4& aTargetMtx, const core::Bone2* aTopBone);
    void renderTags(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    class Tag
    {
    public:
        Tag();
        core::ObjectNode* node;
        Tag* parent;
        QList<Tag> children;
        QRectF originRect;
        QRectF sortedRect;
        bool isDir;
        bool isOpened;
        bool invisibleTop() const { return !parent; }
    };

    static bool compareNodeTagHeight(Tag* a, Tag* b);
    void resetTags(core::ObjectNode& aNode, Tag& aTag);
    void setGeometryRecursive(Tag& aTag);
    QRectF getNodeRectF(core::ObjectNode& aNode) const;
    bool updateIntersection(Tag& aTag, const QPointF& aPos);
    void renderOneNode(const Tag& aTag, QPixmap& aIconPix, int aColorType,
                       const core::RenderInfo& aInfo, QPainter& aPainter);
    Tag* findVisibleTag(const core::ObjectNode& aNode) const;

    const GraphicStyle& mGraphicStyle;
    core::ObjectNode& mTopNode;
    Tag mTopTag;
    Tag* mCurrentTopTag;
    Tag* mCurrentFocus;
    Tag* mCurrentSelect;
    bool mIconFocused;
    bool mFocusChanged;
    std::vector<Tag*> mSortVector;
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_NODESELECTOR_H
