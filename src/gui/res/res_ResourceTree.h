#ifndef GUI_RES_RESOURCETREE_H
#define GUI_RES_RESOURCETREE_H

#include <QTreeWidget>
#include <QAction>
#include "util/Signaler.h"
#include "cmnd/Stack.h"
#include "img/PSDFormat.h"
#include "core/ResourceHolder.h"
#include "core/Project.h"
#include "gui/ViaPoint.h"
#include "gui/res/res_Notifier.h"

namespace gui {
namespace res {

class ResourceTree : public QTreeWidget
{
    Q_OBJECT
public:
    enum { kItemColumn = 0 };
    enum { kColumnCount = 1 };
    typedef QList<img::ResourceNode*> NodeList;

    ResourceTree(ViaPoint& aViaPoint, bool aUseCustomContext, QWidget* aParent);
    void setProject(core::Project* aProject);

    void resetTreeView();
    void resetTreeView(core::ResourceHolder& aHolder);
    void resetTreeView(core::ResourceHolder& aHolder, const util::TreePos& aRoot);
    void updateTreeRootName(core::ResourceHolder& aHolder);

    util::Signaler<void(const NodeList&)> onNodeSelectionChanged;

private:
    QTreeWidgetItem* findItem(const util::TreePos& aPos);
    void addTreeItemRecursive(QTreeWidgetItem* aItem, img::ResourceNode* aNode);
    QList<img::ResourceNode*> findSelectingNodes() const;
    void onItemSelectionChanged();
    void onContextMenuRequested(const QPoint& aPos);
    void onChangePathActionTriggered(bool aIsTriggered);
    void onReloadActionTriggered(bool aIsTriggered);
    void onDeleteActionTriggered(bool aIsTriggered);

    ViaPoint& mViaPoint;
    core::Project* mProject;
    core::ResourceHolder* mHolder;
    QTreeWidgetItem* mActionItem;
    QAction* mChangePathAction;
    QAction* mReloadAction;
    QAction* mDeleteAction;
};

} // namespace res
} // namespace gui

#endif // GUI_RES_RESOURCETREE_H
