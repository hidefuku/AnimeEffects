#include "gui/obj/obj_Util.h"

namespace gui
{
namespace obj
{

QTreeWidgetItem* Util::findItem(gui::ObjectTreeWidget& aTree, const util::TreePos& aPos)
{
    QTreeWidgetItem* current = aTree.invisibleRootItem();
    XC_PTR_ASSERT(current);
    if (!aPos.isValid()) return NULL;

    for (int i = 0; i < aPos.depth() - 1; ++i)
    {
        current = current->child(aPos.row(i));
        if (!current) return NULL;
    }
    QTreeWidgetItem* item = current->child(aPos.tailRow());
    return item;
}

QTreeWidgetItem* Util::removeItem(gui::ObjectTreeWidget& aTree, const util::TreePos& aPos)
{
    QTreeWidgetItem* current = aTree.invisibleRootItem();
    XC_PTR_ASSERT(current);
    XC_ASSERT(aPos.isValid());
    if (!current || !aPos.isValid()) return NULL; // fail safe

    for (int i = 0; i < aPos.depth() - 1; ++i)
    {
        current = current->child(aPos.row(i));
        XC_PTR_ASSERT(current);
    }
    QTreeWidgetItem* item = current->child(aPos.tailRow());
    current->removeChild(item);
    return item;
}

void Util::insertItem(gui::ObjectTreeWidget& aTree, const util::TreePos& aPos, QTreeWidgetItem& aItem)
{
    QTreeWidgetItem* current = aTree.invisibleRootItem();
    XC_PTR_ASSERT(current);
    XC_ASSERT(aPos.isValid());
    if (!current || !aPos.isValid()) return; // fail safe

    for (int i = 0; i < aPos.depth() - 1; ++i)
    {
        current = current->child(aPos.row(i));
        XC_PTR_ASSERT(current);
    }
    current->insertChild(aPos.tailRow(), &aItem);
}

} // namespace obj
} // namespace gui
