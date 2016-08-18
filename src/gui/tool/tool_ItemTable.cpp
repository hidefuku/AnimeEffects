#include "gui/tool/tool_ItemTable.h"

namespace gui {
namespace tool {

ItemTable::ItemTable(const QPoint& aLTPos, int aWidth,
                     const QSize& aItemSize, const QSize& aGap)
    : mLTPos(aLTPos)
    , mWidth(aWidth)
    , mItemSize(aItemSize)
    , mColumn()
    , mRowIndex(0)
    , mColIndex(0)
    , mGap(aGap)
{
    mColumn = std::max(1, mWidth / (mItemSize.width() + mGap.width()));
}

void ItemTable::pushGeometry(QWidget& aItem)
{
    if (mColIndex >= mColumn)
    {
        mColIndex = 0;
        ++mRowIndex;
    }

    const int x = mColIndex * (mItemSize.width() + mGap.width()) + mLTPos.x();
    const int y = mRowIndex * (mItemSize.height() + mGap.height()) + mLTPos.y();
    aItem.setGeometry(x, y, mItemSize.width(), mItemSize.height());

    ++mColIndex;
}

int ItemTable::height() const
{
    return (mRowIndex + 1) * (mItemSize.height() + mGap.height()) - mGap.height();
}

} // namespace tool
} // namespace gui
