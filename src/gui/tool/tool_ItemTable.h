#ifndef GUI_TOOL_ITEMTABLE_H
#define GUI_TOOL_ITEMTABLE_H

#include <QWidget>

namespace gui {
namespace tool {

class ItemTable
{
public:
    ItemTable(const QPoint& aLTPos, int aWidth,
              const QSize& aItemSize, const QSize& aGap = QSize());
    void pushGeometry(QWidget& aItem);
    int height() const;

private:
    QPoint mLTPos;
    int mWidth;
    QSize mItemSize;
    int mColumn;
    int mRowIndex;
    int mColIndex;
    QSize mGap;
};

} // namespace tool
} // namespace gui

#endif // GUI_TOOL_ITEMTABLE_H
