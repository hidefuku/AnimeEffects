#include "gui/obj/obj_InsertItem.h"

namespace gui
{
namespace obj
{

InsertItem::InsertItem(QTreeWidgetItem& aParent, int aIndex, Item& aItem)
    : mParent(aParent)
    , mItem(aItem)
    , mIndex(aIndex)
{
}

void InsertItem::undo()
{
    mParent.removeChild(&mItem);
}

void InsertItem::redo()
{
    mParent.insertChild(mIndex, &mItem);
}

} // namespace obj
} // namespace gui

