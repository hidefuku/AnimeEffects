#include "gui/obj/obj_MoveItem.h"
#include "gui/obj/obj_Util.h"

namespace gui {
namespace obj {

//-------------------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------------------
MoveItems::MoveItems(ObjectTreeWidget& aTree,
                     const Positions& aRemoved,
                     const Positions& aInserted)
    : mTree(aTree)
    , mRemoved(aRemoved)
    , mInserted(aInserted)
{
}

void MoveItems::exec()
{
    // QTreeWidget executed already
}

void MoveItems::undo()
{
    QVector<QTreeWidgetItem*> items;
    for (auto itr = mInserted.rbegin(); itr != mInserted.rend(); ++itr)
    {
        QTreeWidgetItem* item = Util::removeItem(mTree, *itr);
        XC_PTR_ASSERT(item);
        items.push_back(item);
    }
    {
        auto itemItr = items.begin();
        for (auto itr = mRemoved.rbegin(); itr != mRemoved.rend(); ++itr)
        {
            Util::insertItem(mTree, *itr, *(*itemItr));
            ++itemItr;
        }
    }
}

void MoveItems::redo()
{
    QVector<QTreeWidgetItem*> items;
    for (auto itr = mRemoved.begin(); itr != mRemoved.end(); ++itr)
    {
        QTreeWidgetItem* item = Util::removeItem(mTree, *itr);
        XC_PTR_ASSERT(item);
        items.push_back(item);
    }
    {
        auto itemItr =items.begin();
        for (auto itr = mInserted.begin(); itr != mInserted.end(); ++itr)
        {
            Util::insertItem(mTree, *itr, *(*itemItr));
            ++itemItr;
        }
    }
}

} // namespace obj
} // namespace gui

