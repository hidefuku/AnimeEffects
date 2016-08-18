#include "gui/obj/obj_RemoveItem.h"

namespace gui
{
namespace obj
{

RemoveItem::RemoveItem(QTreeWidgetItem& aParent, int aIndex, Item& aItem)
    : mParent(aParent)
    , mItem(aItem)
    , mIndex(aIndex)
{
}

void RemoveItem::undo()
{
    mParent.insertChild(mIndex, &mItem);
}

void RemoveItem::redo()
{
    mParent.removeChild(&mItem);
}

} // namespace obj
} // namespace gui

