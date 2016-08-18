#include "gui/res/res_ResourceTree.h"
#include "gui/res/res_Item.h"

namespace
{
static const int kItemColumn = gui::res::ResourceTree::kItemColumn;
static const int kItemSize = 22;
}

namespace gui {
namespace res {

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

} // namespace res
} // namespace gui
