#ifndef GUI_OBJ_MOVEITEM_H
#define GUI_OBJ_MOVEITEM_H

#include "util/TreePos.h"
#include "cmnd/Stable.h"
#include "gui/ObjectTreeWidget.h"

namespace gui
{
namespace obj
{

class MoveItem : public cmnd::Stable
{
public:
    MoveItem(ObjectTreeWidget& aTree, const util::TreePos& aFrom, const util::TreePos& aTo);
    virtual void exec();
    virtual void undo();
    virtual void redo();

private:
    ObjectTreeWidget& mTree;
    util::TreePos mFrom;
    util::TreePos mTo;
};

class MoveItems : public cmnd::Stable
{
public:
    typedef QVector<util::TreePos> Positions;
    MoveItems(ObjectTreeWidget& aTree,
              const Positions& aRemoved,
              const Positions& aInserted);
    virtual void exec();
    virtual void undo();
    virtual void redo();

private:
    ObjectTreeWidget& mTree;
    Positions mRemoved;
    Positions mInserted;
};

} // namespace obj
} // namespace gui

#endif // GUI_OBJ_MOVEITEM_H
