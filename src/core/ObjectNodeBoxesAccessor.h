#ifndef CORE_OBJECTNODEBOXESACCESSOR_H
#define CORE_OBJECTNODEBOXESACCESSOR_H

#if 0
#include <QMultiMap>
#include <QVector>
#include <QPainter>
#include "core/CameraInfo.h"
#include "core/RenderInfo.h"
#include "core/ObjectNodeBox.h"
#include "core/GraphicsMetrics.h"

namespace core
{

class ObjectNode;

class ObjectNodeBoxesAccessor
{
public:
    ObjectNodeBoxesAccessor(const GraphicsMetrics& aGraphicsMetrics);
    ~ObjectNodeBoxesAccessor();

    void update(const CameraInfo& aCameraInfo, ObjectNode* aTopNode);
    void draw(const RenderInfo& aInfo, QPainter& aPainter);
    void drawBindArrow(const RenderInfo& aInfo, QPainter& aPainter, const ObjectNode* aNode, const QVector2D& aScreenPos, bool aIsFocus);

    ObjectNode* find(const QVector2D& aScreenPos);
    void setFocus(ObjectNode* aNode);
    QVector2D arrowRootPos(const ObjectNode* aNode) const;

private:
    static void drawNodeBox(const RenderInfo& aInfo, QPainter& aPainter, const ObjectNode* aNode);
    static void drawNodeBoxLabel(const RenderInfo& aInfo, QPainter& aPainter, const ObjectNode* aNode);

    void updateLabelGeometry(const ObjectNode* aNode, const CameraInfo& aCameraInfo, QMultiMap<int, QRectF>& aHeightSortLabels, bool aRecursive);
    void drawBindArrow(const RenderInfo& aInfo, QPainter& aPainter, const ObjectNode* aNode, bool aIsFocus);
    void drawBindArrow(const RenderInfo& aInfo, QPainter& aPainter, const ObjectNode* aNode, const QVector2D& aScreenPos, const QColor& aColor, const QColor& aShadowColor);

    const GraphicsMetrics& mGraphicsMetrics;
    ObjectNode* mTopNode;
};

} // namespace core
#endif

#endif // CORE_OBJECTNODEBOXESACCESSOR_H
