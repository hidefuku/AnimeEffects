#ifndef GUI_OBJ_REMOVEITEM_H
#define GUI_OBJ_REMOVEITEM_H

#include <QTreeWidgetItem>
#include "cmnd/Stable.h"
#include "gui/obj/obj_Item.h"

namespace gui
{
namespace obj
{

class RemoveItem : public cmnd::Stable
{
    QTreeWidgetItem& mParent;
    Item& mItem;
    int mIndex;
public:
    RemoveItem(QTreeWidgetItem& aParent, int aIndex, Item& aItem);

    virtual void undo();
    virtual void redo();
};

} // namespace obj
} // namespace gui

#endif // GUI_OBJ_REMOVEITEM_H
