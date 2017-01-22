#ifndef GUI_OBJ_ITEM_H
#define GUI_OBJ_ITEM_H

#include <QTreeWidget>
#include "core/ObjectNode.h"

namespace gui {
namespace obj {

class Item : public QTreeWidgetItem
{
public:
    static Item* cast(QTreeWidgetItem* aItem);
    static const Item* cast(const QTreeWidgetItem* aItem);

    Item(const QTreeWidget& aTree, core::ObjectNode& aNode);

    core::ObjectNode& node() { return mNode; }
    const core::ObjectNode& node() const { return mNode; }

    bool isTopNode() const;

    QRect visualRect() const;

private:
    const QTreeWidget& mTree;
    core::ObjectNode& mNode;
};

} // namespace obj
} // namespace gui


#endif // GUI_OBJ_ITEM_H
