#include "gui/res/res_ResourceTree.h"
#include "gui/res/res_Item.h"

namespace
{
static const int kItemColumn = gui::res::ResourceTree::kItemColumn;
static const int kItemSize = 22;

//-------------------------------------------------------------------------------------------------
void pushTreeRowRecursive(
        util::TreePos& aDst, const QTreeWidget& aTree, QTreeWidgetItem* aObj)
{
    if (!aObj) return;

    QTreeWidgetItem* parent = aObj->parent();
    if (parent)
    {
        pushTreeRowRecursive(aDst, aTree, parent);
        const int index = parent->indexOfChild(aObj);
        XC_ASSERT(index >= 0);
        aDst.pushRow(index);
    }
    else
    {
        const int index = aTree.indexOfTopLevelItem(aObj);
        XC_ASSERT(index >= 0);
        aDst.pushRow(index);
    }
}

util::TreePos getTreePos(QTreeWidgetItem* aObj)
{
    util::TreePos pos;
    if (!aObj) return pos;

    const QTreeWidget* tree = aObj->treeWidget();
    if (!tree) return pos;

    pos.setValidity((bool)aObj);
    pushTreeRowRecursive(pos, *tree, aObj);
    return pos;
}

}

namespace gui {
namespace res {

//-------------------------------------------------------------------------------------------------
Item* Item::cast(QTreeWidgetItem* aItem)
{
    return (aItem->whatsThis(kItemColumn) == "ResourceItem") ?
                static_cast<Item*>(aItem) : nullptr;
}

const Item* Item::cast(const QTreeWidgetItem* aItem)
{
    return (aItem->whatsThis(kItemColumn) == "ResourceItem") ?
                static_cast<const Item*>(aItem) : nullptr;
}

Item::Item(const QTreeWidget& aTree, img::ResourceNode& aNode, const QString& aIdentifier)
    : QTreeWidgetItem()
    , mTree(aTree)
    , mNode(aNode)
{
    this->setWhatsThis(kItemColumn, "ResourceItem");
    this->setText(kItemColumn, aIdentifier);
    this->setSizeHint(kItemColumn, QSize(kItemSize, kItemSize));
}

util::TreePos Item::treePos()
{
    return getTreePos(this);
}

} // namespace res
} // namespace gui
