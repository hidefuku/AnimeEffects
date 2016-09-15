#if 0
#include "core/ObjectNodeBoxesAccessor.h"
#include "core/ObjectNode.h"
#include "util/MathUtil.h"
#include "core/ObjectNodeUtil.h"

namespace core
{

void ObjectNodeBoxesAccessor::drawNodeBox(const RenderInfo& aInfo, QPainter& aPainter, const ObjectNode* aNode)
{
    (void)aInfo;
    (void)aPainter;
    (void)aNode;

#if 0
    if (aNode && aNode->boxCache() && aNode->boxCache()->rect.isValid())
    {
        const ObjectNodeBox& box = *aNode->boxCache();
        const bool isBind = aNode->skeleton() && aNode->skeleton()->isBound();

        QColor color(60, 60, 60, box.isPack ? 200 : 130);
        QColor shadowColor(255, 255, 255, 45);

        if (box.isFocusing)
        {
            color = QColor(250, 250, 250, 250);
            shadowColor = QColor(50, 50, 50, 45);
        }
        else if (isBind)
        {
            color = QColor(60, 60, 150, 200);
            shadowColor = QColor(255, 255, 255, 45);
        }

        //const QPointF kHandleSize(8.0f, 8.0f);
        const Qt::PenStyle penStyle = box.isPack ? Qt::DashLine : Qt::SolidLine;

        QRectF rect = aInfo.camera.toScreenRectF(box.rect);
        QVector<QLineF> outline;
        outline.push_back(QLineF(rect.topLeft(), rect.topRight()));
        outline.push_back(QLineF(rect.bottomLeft(), rect.bottomRight()));
        outline.push_back(QLineF(rect.topLeft(), rect.bottomLeft()));
        outline.push_back(QLineF(rect.topRight(), rect.bottomRight()));

        aPainter.setBrush(QBrush());
        aPainter.setPen(QPen(QBrush(shadowColor), 3, Qt::SolidLine));
        aPainter.drawLines(outline);
        aPainter.setPen(QPen(QBrush(color), 1, penStyle));
        aPainter.drawLines(outline);
        //aPainter.drawRect(QRectF(rect.topLeft() - kHandleSize, rect.topLeft()));
        //aPainter.drawRect(QRectF(rect.bottomRight(), rect.bottomRight() + kHandleSize));
    }
#endif
}

void ObjectNodeBoxesAccessor::drawNodeBoxLabel(const RenderInfo& aInfo, QPainter& aPainter, const ObjectNode* aNode)
{
    (void)aInfo;
    (void)aPainter;
    (void)aNode;
#if 0
    if (aNode && aNode->boxCache() && aNode->boxCache()->labelRect.isValid())
    {
        const ObjectNodeBox& box = *aNode->boxCache();
        const bool isBind = aNode->skeleton() && aNode->skeleton()->isBound();

        QColor textColor(240, 240, 240, 250);
        QColor backColor(100, 100, 100, 200);

        if (box.isFocusing)
        {
            textColor = QColor(50, 50, 50, 255);
            backColor = QColor(250, 250, 250, 220);
        }
        else if (isBind)
        {
            backColor = QColor(80, 80, 140, 200);
        }

        aPainter.setBrush(QBrush(backColor));
        aPainter.setPen(QPen(QBrush(backColor), 0));
        aPainter.drawRect(box.labelRect);

        aPainter.setPen(QPen(QBrush(textColor), 1));
        aPainter.drawText(box.labelRect.translated(5, 0), aNode->name());
    }
#endif
}

ObjectNodeBoxesAccessor::ObjectNodeBoxesAccessor(const GraphicsMetrics& aGraphicsMetrics)
    : mGraphicsMetrics(aGraphicsMetrics)
    , mTopNode()
{
}

ObjectNodeBoxesAccessor::~ObjectNodeBoxesAccessor()
{
}

void ObjectNodeBoxesAccessor::update(const CameraInfo& aCameraInfo, ObjectNode* aTopNode)
{
    mTopNode = aTopNode;

    if (mTopNode)
    {
        ObjectNodeUtil::buildBoundingBox(*mTopNode);

        QMultiMap<int, QRectF> heightSortLabels;
        updateLabelGeometry(mTopNode, aCameraInfo, heightSortLabels, false);
    }
}

ObjectNode* ObjectNodeBoxesAccessor::find(const QVector2D& aScreenPos)
{
    ObjectNode::Iterator itr(mTopNode);
    while (itr.hasNext())
    {
        ObjectNode* node = itr.next();
        if (node && node->boxCache())
        {
            if (node->boxCache()->labelRect.contains(aScreenPos.toPointF()))
            {
                return node;
            }
        }
    }
    return NULL;
}

void ObjectNodeBoxesAccessor::setFocus(ObjectNode* aNode)
{
    ObjectNode::Iterator itr(mTopNode);
    while (itr.hasNext())
    {
        ObjectNode* node = itr.next();
        if (node && node->boxCache())
        {
            node->boxCache()->isFocusing = node == aNode ? true : false;
        }
    }
}

void ObjectNodeBoxesAccessor::updateLabelGeometry(const ObjectNode* aNode, const CameraInfo& aCameraInfo, QMultiMap<int, QRectF>& aHeightSortLabels, bool aRecursive)
{
    if (!aNode) return;
    for (auto node : aNode->children())
    {
        if (aRecursive)
        {
            updateLabelGeometry(node, aCameraInfo, aHeightSortLabels, true);
        }

        if (node && node->boxCache() && node->isVisible())
        {
            QRectF labelRect = node->boxCache()->labelRect;
            labelRect = mGraphicsMetrics.boundingRect(node->name());
            labelRect.setWidth(labelRect.width() + 10);
            labelRect.translate(-labelRect.topLeft());
            labelRect = labelRect.translated(aCameraInfo.toScreenPos(node->boxCache()->rect.topLeft()));

            for (auto lr : aHeightSortLabels)
            {
                if (lr.intersects(labelRect))
                {
                    labelRect.moveTop(lr.bottom() + 1);
                }
            }
            aHeightSortLabels.insert(labelRect.top(), labelRect);
            node->boxCache()->labelRect = labelRect;
        }
    }
}

void ObjectNodeBoxesAccessor::draw(const RenderInfo& aInfo, QPainter& aPainter)
{
    for (ObjectNode::ConstIterator itr = mTopNode; itr.hasNext();)
    {
        const ObjectNode* node = itr.next();
        if (node->parent() != mTopNode) continue;
        if (node && node->boxCache() && !node->boxCache()->isFocusing)
        {
            drawBindArrow(aInfo, aPainter, node, false);
            drawNodeBox(aInfo, aPainter, node);
        }
    }

    for (ObjectNode::ConstIterator itr = mTopNode; itr.hasNext();)
    {
        const ObjectNode* node = itr.next();
        if (node->parent() != mTopNode) continue;
        if (node && node->boxCache() && !node->boxCache()->isFocusing)
        {
            drawNodeBoxLabel(aInfo, aPainter, node);
        }
    }

    for (ObjectNode::ConstIterator itr = mTopNode; itr.hasNext();)
    {
        const ObjectNode* node = itr.next();
        if (node->parent() != mTopNode) continue;
        if (node && node->boxCache() && node->boxCache()->isFocusing)
        {
            drawBindArrow(aInfo, aPainter, node, true);
            drawNodeBox(aInfo, aPainter, node);
            drawNodeBoxLabel(aInfo, aPainter, node);
        }
    }
}

void ObjectNodeBoxesAccessor::drawBindArrow(const RenderInfo& aInfo, QPainter& aPainter, const ObjectNode* aNode, const QVector2D& aScreenPos, bool aIsFocus)
{
    QColor color(50, 50, 150, 150);
    QColor shadowColor(240, 240, 240, 30);

    if (aIsFocus)
    {
        color = QColor(255, 255, 255, 220);
        shadowColor = QColor(50, 50, 50, 35);
    }

    drawBindArrow(aInfo, aPainter, aNode, aScreenPos, color, shadowColor);
}

void ObjectNodeBoxesAccessor::drawBindArrow(const RenderInfo& aInfo, QPainter& aPainter, const ObjectNode* aNode, bool aIsFocus)
{
    (void)aInfo;
    (void)aPainter;
    (void)aNode;
    (void)aIsFocus;
#if 0
    if (aNode && aNode->skeleton() && aNode->skeleton()->isBound() && aNode->skeleton()->isBindVisible())
    {
        const Bone* bone = aNode->skeleton()->bind();
        QVector2D pos = aNode->skeleton()->bind()->pos2D();

        if (bone->parent())
        {
            pos = 0.5f * (pos + bone->parent()->pos2D());
        }

        drawBindArrow(aInfo, aPainter, aNode, aInfo.camera.toScreenPos(pos), aIsFocus);
    }
#endif
}

void ObjectNodeBoxesAccessor::drawBindArrow(const RenderInfo& aInfo, QPainter& aPainter, const ObjectNode* aNode, const QVector2D& aScreenPos, const QColor& aColor, const QColor& aShadowColor)
{
    (void)aInfo;
    if (aNode && aNode->boxCache())
    {
        const QBrush kBrush(aColor);
        const QBrush kBrushShadow(aShadowColor);

        const QPointF from(aNode->boxCache()->labelRect.center());
        const QPointF to(aScreenPos.toPointF());
        const QVector2D dir = QVector2D(to - from);
        if (dir.length() < 0.01f) return;

        const QVector2D arrowN = dir.normalized();
        const float arrowLength = std::min(0.1f * dir.length(), 15.0f);
        const QPointF arrowV((arrowLength * arrowN).toPointF());
        const QPointF arrowH(0.25f * util::MathUtil::getRotateVector90Deg(QVector2D(arrowV)).toPointF());

        aPainter.setBrush(kBrush);
        aPainter.setPen(QPen(kBrushShadow, 3));
        aPainter.drawLine(from, to);

        aPainter.setPen(QPen(kBrush, 1, Qt::DashLine));
        aPainter.drawLine(from, to);

        aPainter.setPen(QPen(kBrushShadow, 1));
        QPointF arrow[3];
        arrow[0] = to;
        arrow[1] = to - arrowV + arrowH;
        arrow[2] = to - arrowV - arrowH;
        aPainter.drawConvexPolygon(arrow, 3);
    }
}

} // namespace core
#endif
