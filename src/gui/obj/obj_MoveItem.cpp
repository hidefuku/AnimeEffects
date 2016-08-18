#include "gui/obj/obj_MoveItem.h"
#include "gui/obj/obj_Util.h"

//-------------------------------------------------------------------------------------------------
namespace gui
{
namespace obj
{

MoveItem::MoveItem(ObjectTreeWidget& aTree, const util::TreePos& aFrom, const util::TreePos& aTo)
    : mTree(aTree)
    , mFrom(aFrom)
    , mTo(aTo)
{
}

void MoveItem::exec()
{
    // QTreeWidget executed already
}

void MoveItem::undo()
{
    QTreeWidgetItem* item = Util::removeItem(mTree, mTo);
    XC_PTR_ASSERT(item);
    Util::insertItem(mTree, mFrom, *item);
}

void MoveItem::redo()
{
    QTreeWidgetItem* item = Util::removeItem(mTree, mFrom);
    XC_PTR_ASSERT(item);
    Util::insertItem(mTree, mTo, *item);
}

} // namespace obj
} // namespace gui

