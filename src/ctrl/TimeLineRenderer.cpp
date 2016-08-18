#include "ctrl/TimeLineRenderer.h"

using namespace core;

namespace ctrl
{

TimeLineRenderer::TimeLineRenderer(QPainter& aPainter, const core::CameraInfo& aCamera)
    : mPainter(aPainter)
    , mCamera(aCamera)
    , mMargin()
    , mRange()
    , mScale()
{
}

void TimeLineRenderer::renderLines(const QVector<TimeLineRow>& aRows, const QRect& aCameraRect, const QRect& aCullRect)
{
    // draw each line
    mPainter.setRenderHint(QPainter::Antialiasing);

    const QBrush kBrushBody(QColor(250, 250, 250, 255));
    const QBrush kBrushBodySelect(QColor(235, 240, 250, 255));
    const QBrush kBrushEdge(QColor(190, 190, 190, 255));
    const QBrush kBrushSepa(QColor(200, 200, 205, 255));
    const QBrush kBrushText(QColor(170, 170, 170, 255));
    const int textWidth = 100;
    const int textLeft = aCameraRect.center().x() - textWidth / 2;

    for (const TimeLineRow& row : aRows)
    {
        const QRect rect = row.rect;
        const QPoint cpos(mMargin, rect.bottom());

        if (!aCullRect.intersects(rect)) continue;

        // draw line
        mPainter.setPen(QPen(kBrushEdge, 1));
        mPainter.setBrush(row.selecting ? kBrushBodySelect : kBrushBody);
        mPainter.drawRect(rect);

        if (row.node && row.node->timeLine())
        {
            mPainter.setPen(QPen(kBrushSepa, 1, Qt::DotLine));

            const int sepa = row.node->timeLine()->validTypeCount();

            for (int i = 1; i < sepa; ++i)
            {
                const float h = (float)rect.height() / sepa;
                const float y = rect.top() + i * h;
                const QPointF v0(rect.left(), y);
                const QPointF v1(rect.right(), y);
                mPainter.drawLine(v0, v1);

            }

            mPainter.setPen(QPen(kBrushText, 1));
            int i = 0;
            for (int typei = 0; typei < core::TimeKeyType_TERM; ++typei)
            {
                auto type = (core::TimeKeyType)typei;
                if (row.node->timeLine()->isEmpty(type))
                {
                    continue;
                }

                const float h = (float)rect.height() / sepa;
                mPainter.drawText(
                            QRect(textLeft, rect.top() + (int)i * h,
                                  textWidth, (int)h),
                            core::TimeLine::timeKeyName(type),
                            QTextOption(Qt::AlignCenter));
                ++i;
            }
        }

        // draw child keys
        if (row.closedFolder)
        {
            drawChildKeys(row.node, cpos);
        }
        // draw keys
        drawKeys(row.node, row);
    }
}

void TimeLineRenderer::renderHeader(int aHeight, int aFps)
{
    const QRect cameraRect(-mCamera.pos().toPoint(), mCamera.screenSize());

    mPainter.setRenderHint(QPainter::Antialiasing, false);

    // draw header background
    {
        const QBrush kBrush(QColor(160, 160, 160, 255));
        QRect rect = cameraRect;
        rect.setHeight(aHeight);
        mPainter.setPen(QPen(kBrush, 1));
        mPainter.setBrush(kBrush);
        mPainter.drawRect(rect);
    }

    // draw header info
    {
        const QBrush kBrush(QColor(60, 60, 70, 255));
        const int numberHeight = 14;
        const int numberWidth = 6;
        const QPoint lt(mMargin, cameraRect.top());
        const QPoint rb = lt + QPoint(mScale->maxPixelWidth(), aHeight);

        mPainter.setPen(QPen(kBrush, 1));

        for (int i = mRange.min(); i <= mRange.max(); ++i)
        {
            auto attr = mScale->attribute(i);

            QPoint pos(lt.x() + attr.grid.x(), rb.y());
            mPainter.drawLine(pos, pos + QPoint(0, -attr.grid.y()));

            if (attr.showNumber)
            {
                QString number;
                number.sprintf("%.1f", (float)i / aFps);
                const int width = numberWidth * number.size();
                const int left = pos.x() - (width >> 1);
                const int top = lt.y() - 1;
                const QRect rect(QPoint(left, top), QPoint(left + width + 1, top + numberHeight));
                mPainter.drawText(rect, number);
            }
        }
    }
}

void TimeLineRenderer::renderHandle(const QPoint& aPoint, int aRange)
{
    const QPoint pos = aPoint + QPoint(0, -(int)mCamera.pos().y());
    const int range = aRange;

    const QBrush kBrushBody(QColor(230, 230, 230, 180));
    const QBrush kBrushEdge(QColor(120, 120, 120, 180));

    mPainter.setPen(QPen(kBrushEdge, 1));
    mPainter.setBrush(kBrushBody);
    mPainter.drawLine(pos + QPoint(0, range), pos + QPoint(0, mCamera.screenHeight()));

    mPainter.setRenderHint(QPainter::Antialiasing);
    mPainter.drawEllipse(pos, range, range);
}

void TimeLineRenderer::renderSelectionRange(const QRect& aRect)
{
    if (aRect.width() >= 2 && aRect.height() >= 2)
    {
        mPainter.setRenderHint(QPainter::Antialiasing, false);
        const QBrush kSelectEdge(QColor(140, 140, 140, 128));
        const QBrush kSelectBody(QColor(0, 0, 255, 16));
        mPainter.setPen(QPen(kSelectEdge, 1, Qt::DashLine));
        mPainter.setBrush(kSelectBody);
        mPainter.drawRect(aRect);
    }
}

void TimeLineRenderer::drawKeys(const ObjectNode* aNode, const TimeLineRow& aRow)
{
    const QBrush kBrushKeyBody1(QColor(145, 145, 145, 255));
    const QBrush kBrushKeyBody2(QColor(240, 240, 240, 255));
    const QBrush kBrushKeyEdge(QColor(90, 90, 100, 255));

    if (aNode && aNode->timeLine())
    {
        const TimeLine& timeLine = *(aNode->timeLine());
        const int validNum = timeLine.validTypeCount();
        const int left = aRow.rect.left();
        int validIndex = 0;

        for (int i = 0; i < TimeKeyType_TERM; ++i)
        {
            const TimeLine::MapType& map = timeLine.map((TimeKeyType)i);
            if (map.isEmpty()) continue;

            const float height = aRow.keyHeight(validIndex, validNum);
            ++validIndex;

            mPainter.setPen(QPen(kBrushKeyEdge, 1));

            auto itr = map.lowerBound(mRange.min());
            while (itr != map.end() && itr.key() <= mRange.max())
            {
                const bool isFocused = itr.value()->isFocused();
                mPainter.setBrush(isFocused ? kBrushKeyBody2 : kBrushKeyBody1);

                auto attr = mScale->attribute(itr.key());
                QPointF pos(left + attr.grid.x() + 0.5f, height + 0.5f);

                if (itr.value()->canHoldChild())
                {
                    QPointF offsx(4.2f, 0.0f);
                    QPointF offsy(0.0f, 4.2f);
                    QPointF poly[4] = {
                        pos - offsy, pos + offsx,
                        pos + offsy, pos - offsx
                    };
                    mPainter.drawConvexPolygon(poly, 4);
                }
                else
                {
                    mPainter.drawEllipse(pos, 3.0f, 3.0f);
                }

                ++itr;
            }
        }
    }
}

void TimeLineRenderer::drawChildKeys(const ObjectNode* aNode, const QPoint& aPos)
{
    const QBrush kBrushKey(QColor(170, 170, 170, 255));

    mPainter.setPen(QPen(kBrushKey, 1));
    mPainter.setBrush(kBrushKey);

    for (auto child : aNode->children())
    {
        ObjectNode::ConstIterator treeItr(child);
        while (treeItr.hasNext())
        {
            const ObjectNode* node = treeItr.next();
            if (!node->timeLine()) continue;
            const TimeLine& timeLine = *(node->timeLine());

            for (int i = 0; i < TimeKeyType_TERM; ++i)
            {
                const TimeLine::MapType& map = timeLine.map((TimeKeyType)i);

                auto itr = map.lowerBound(mRange.min());
                while (itr != map.end() && itr.key() <= mRange.max())
                {
                    auto attr = mScale->attribute(itr.key());
                    QPointF pos[3];
                    pos[0] = QPointF(aPos.x() + attr.grid.x() + 0.5f, aPos.y());
                    pos[1] = pos[0] + QPointF( 3, -5);
                    pos[2] = pos[0] + QPointF(-3, -5);
                    mPainter.drawConvexPolygon(pos, 3);
                    ++itr;
                }
            }
        }
    }
}

} // namespace ctrl
