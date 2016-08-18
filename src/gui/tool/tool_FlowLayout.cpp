#include <QWidget>
#include "gui/tool/tool_FlowLayout.h"

namespace gui {
namespace tool {

FlowLayout::FlowLayout(QWidget* aParent, int aMargin, int aHSpacing, int aVSpacing)
    : QLayout(aParent)
    , mHSpace(aHSpacing)
    , mVSpace(aVSpacing)
{
    this->setContentsMargins(aMargin, aMargin, aMargin, aMargin);
}

FlowLayout::FlowLayout(int aMargin, int aHSpacing, int aVSpacing)
    : mHSpace(aHSpacing)
    , mVSpace(aVSpacing)
{
    this->setContentsMargins(aMargin, aMargin, aMargin, aMargin);
}

FlowLayout::~FlowLayout()
{
    for (auto item : mItemList)
    {
        delete item;
    }
}

void FlowLayout::addItem(QLayoutItem* aItem)
{
    mItemList.append(aItem);
}

int FlowLayout::horizontalSpacing() const
{
    return mHSpace >= 0 ?
                mHSpace :
                smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::verticalSpacing() const
{
    return mVSpace >= 0 ?
                mVSpace :
                smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

int FlowLayout::count() const
{
    return mItemList.size();
}

QLayoutItem* FlowLayout::itemAt(int aIndex) const
{
    return mItemList.value(aIndex);
}

QLayoutItem* FlowLayout::takeAt(int aIndex)
{
    if (aIndex < 0 || count() <= aIndex) return nullptr;
    return mItemList.takeAt(aIndex);
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return 0;
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int aWidth) const
{
    const int height = doLayout(QRect(0, 0, aWidth, 0), true);
    return height;
}

void FlowLayout::setGeometry(const QRect& aRect)
{
    QLayout::setGeometry(aRect);
    doLayout(aRect, false);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (auto item : mItemList)
    {
        size = size.expandedTo(item->minimumSize());
    }
    size += QSize(2 * margin(), 2 * margin());
    return size;
}

int FlowLayout::doLayout(const QRect& aRect, bool aTestOnly) const
{
    int left, top, right, bottom;
    this->getContentsMargins(&left, &top, &right, &bottom);

    const QRect effectiveRect = aRect.adjusted(+left, +top, -right, -bottom);

    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int rowHeight = 0;

    for (auto item : mItemList)
    {
        const QSize itemSize = item->sizeHint();

        int spaceX = horizontalSpacing();
        if (spaceX == -1)
        {
            spaceX = item->widget()->style()->layoutSpacing(
                         QSizePolicy::PushButton,
                         QSizePolicy::PushButton,
                         Qt::Horizontal);
        }

        int spaceY = verticalSpacing();
        if (spaceY == -1)
        {
            spaceY = item->widget()->style()->layoutSpacing(
                         QSizePolicy::PushButton,
                         QSizePolicy::PushButton,
                         Qt::Vertical);
        }

        int nextX = x + itemSize.width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && rowHeight > 0)
        {
            x = effectiveRect.x();
            y = y + rowHeight + spaceY;
            nextX = x + itemSize.width() + spaceX;
            rowHeight = 0;
        }

        if (!aTestOnly)
        {
            item->setGeometry(QRect(QPoint(x, y), itemSize));
        }

        x = nextX;
        rowHeight = qMax(rowHeight, itemSize.height());
    }
    return y + rowHeight - aRect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric aMetric) const
{
    QObject* parent = this->parent();
    if (!parent) return -1;

    if (parent->isWidgetType())
    {
        QWidget* pw = static_cast<QWidget*>(parent);
        return pw->style()->pixelMetric(aMetric, 0, pw);
    }
    else
    {
        return static_cast<QLayout*>(parent)->spacing();
    }
}

} // namespace tool
} // namespace gui
