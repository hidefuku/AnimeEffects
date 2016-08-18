#include "gui/ObjectTreeWidget.h"
#include "gui/obj/obj_Item.h"

namespace
{
static const int kItemColumn = gui::ObjectTreeWidget::kItemColumn;
}

namespace gui {
namespace obj {

Item* Item::cast(QTreeWidgetItem* aItem)
{
    if (!aItem) return nullptr;
    return (aItem->whatsThis(kItemColumn) == "ObjectItem") ?
                static_cast<Item*>(aItem) : nullptr;
}

const Item* Item::cast(const QTreeWidgetItem* aItem)
{
    if (!aItem) return nullptr;
    return (aItem->whatsThis(kItemColumn) == "ObjectItem") ?
                static_cast<const Item*>(aItem) : nullptr;
}

Item::Item(const QTreeWidget& aTree, core::ObjectNode& aNode)
    : QTreeWidgetItem()
    , mTree(aTree)
    , mNode(aNode)
{
    this->setText(kItemColumn, aNode.name());
    this->setWhatsThis(kItemColumn, "ObjectItem");
}

QRect Item::visualRect() const
{
    return mTree.visualItemRect(this);
}

} // namespace obj
} // namespace gui
