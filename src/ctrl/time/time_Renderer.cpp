#include "ctrl/time/time_Renderer.h"

using namespace core;

namespace ctrl {
namespace time {

Renderer::Renderer(QPainter& aPainter, const CameraInfo& aCamera)
    : mPainter(aPainter)
    , mCamera(aCamera)
    , mMargin()
    , mRange()
    , mScale()
{
}

void Renderer::renderLines(const QVector<TimeLineRow>& aRows, const QRect& aCameraRect, const QRect& aCullRect)
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

        if (!row.node || !row.node->timeLine()) continue;

        if (!row.node->isSlimmedDown())
        {
            // draw separators
            mPainter.setPen(QPen(kBrushSepa, 1, Qt::DotLine));
            const int sepa = row.node->timeLine()->validTypeCount();
            for (int i = 1; i < sepa; ++i)
            {
                const float h = static_cast<float>(rect.height()) / sepa;
                const float y = rect.top() + i * h;
                const QPointF v0(rect.left(), static_cast<double>(y));
                const QPointF v1(rect.right(), static_cast<double>(y));
                mPainter.drawLine(v0, v1);

            }

            // draw type labels
            mPainter.setPen(QPen(kBrushText, 1));
            int i = 0;
            for (int typei = 0; typei < TimeKeyType_TERM; ++typei)
            {
                auto type = TimeLine::getTimeKeyTypeInOrderOfOperations(typei);
                if (row.node->timeLine()->isEmpty(type))
                {
                    continue;
                }

                const float h = static_cast<float>(rect.height()) / sepa;
                mPainter.drawText(
                            QRect(textLeft, rect.top() + static_cast<int>(i * h), textWidth, static_cast<int>(h)),
                            TimeLine::getTimeKeyName(type),
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

void Renderer::renderHeader(int aHeight, int aFps)
{
    const QRect cameraRect(-mCamera.leftTopPos().toPoint(), mCamera.screenSize());
    const QSettings settings;

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

        const TimeFormat timeFormat(mRange,aFps);
        const TimeFormatType timeFormatVar = static_cast<TimeFormatType>(settings.value("generalsettings/ui/timeformat").toInt());

        mPainter.setPen(QPen(kBrush, 1));

        for (int i = mRange.min(); i <= mRange.max(); ++i)
        {
            auto attr = mScale->attribute(i);

            QPoint pos(lt.x() + attr.grid.x(), rb.y());
            mPainter.drawLine(pos, pos + QPoint(0, -attr.grid.y()));

            if (attr.showNumber)
            {
                QString number = timeFormat.frameToString(i, timeFormatVar);
                const int width = numberWidth * number.size();
                const int left = pos.x() - (width >> 1);
                const int top = lt.y() - 1;
                const QRect rect(QPoint(left, top), QPoint(left + width + 1, top + numberHeight));
                mPainter.drawText(rect, number);
            }
        }
    }
}

void Renderer::renderHandle(const QPoint& aPoint, int aRange)
{
    const QPoint pos = aPoint + QPoint(0, -static_cast<int>(mCamera.leftTopPos().y()));
    const int range = aRange;

    const QBrush kBrushBody(QColor(230, 230, 230, 180));
    const QBrush kBrushEdge(QColor(120, 120, 120, 180));

    mPainter.setPen(QPen(kBrushEdge, 1));
    mPainter.setBrush(kBrushBody);
    mPainter.drawLine(pos + QPoint(0, range), pos + QPoint(0, mCamera.screenHeight()));

    mPainter.setRenderHint(QPainter::Antialiasing);
    mPainter.drawEllipse(pos, range, range);
}

void Renderer::renderSelectionRange(const QRect& aRect)
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

void Renderer::drawKeys(const ObjectNode* aNode, const TimeLineRow& aRow)
{
    const QBrush kBrushKeyBody1(QColor(145, 145, 145, 255));
    const QBrush kBrushKeyBody2(QColor(240, 240, 240, 255));
    const QBrush kBrushKeyEdge(QColor(90, 90, 100, 255));
    QPointF holder[4] = { QPointF(0.0, -4.2), QPointF( 4.2, 0.0),
                          QPointF(0.0,  4.2), QPointF(-4.2, 0.0) };

    if (aNode && aNode->timeLine())
    {
        const TimeLine& timeLine = *(aNode->timeLine());
        const int validNum = timeLine.validTypeCount();
        const int left = aRow.rect.left();
        const bool isSlimmed = aNode->isSlimmedDown();
        int validIndex = 0;

        for (int i = 0; i < TimeKeyType_TERM; ++i)
        {
            auto type = TimeLine::getTimeKeyTypeInOrderOfOperations(i);
            const TimeLine::MapType& map = timeLine.map(type);
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
                QPointF pos(left + attr.grid.x() + 0.5, static_cast<double>(height + 0.5f));

                if (isSlimmed)
                {
                    /*
                    const QPointF poly[3] = {
                        pos + QPointF(0.0f, -2.5f),
                        pos + QPointF(-3.0f, 2.5f),
                        pos + QPointF(3.0f, 2.5f) };
                    mPainter.drawConvexPolygon(poly, 3);
                    */
                    //mPainter.drawEllipse(pos, 3.0f, 1.5f);
                    const QPointF poly[] = {
                        pos + QPointF(-3.0, -2.0),
                        pos + QPointF( 3.0, -2.0),
                        pos + QPointF( 3.0,  2.0),
                        pos + QPointF(-3.0,  2.0) };
                    mPainter.drawConvexPolygon(poly, 4);
                }
                else if (itr.value()->canHoldChild())
                {
                    const QPointF poly[4] = {
                        pos + holder[0], pos + holder[1],
                        pos + holder[2], pos + holder[3] };
                    mPainter.drawConvexPolygon(poly, 4);
                }
                else
                {
                    mPainter.drawEllipse(pos, 3.0, 3.0);
                }

                ++itr;
            }
        }
    }
}

void Renderer::drawChildKeys(const ObjectNode* aNode, const QPoint& aPos)
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
                auto type = TimeLine::getTimeKeyTypeInOrderOfOperations(i);
                const TimeLine::MapType& map = timeLine.map(type);

                auto itr = map.lowerBound(mRange.min());
                while (itr != map.end() && itr.key() <= mRange.max())
                {
                    auto attr = mScale->attribute(itr.key());
                    QPointF pos[3];
                    pos[0] = QPointF(aPos.x() + attr.grid.x() + 0.5, aPos.y());
                    pos[1] = pos[0] + QPointF( 3, -5);
                    pos[2] = pos[0] + QPointF(-3, -5);
                    mPainter.drawConvexPolygon(pos, 3);
                    ++itr;
                }
            }
        }
    }
}

} // namespace time
} // namespace ctrl
