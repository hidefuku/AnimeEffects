#ifndef GUI_RESOURCETREEWIDGET_H
#define GUI_RESOURCETREEWIDGET_H

#include <QTreeWidget>
#include <QAction>
#include "util/Signaler.h"
#include "util/LinkPointer.h"
#include "cmnd/Stack.h"
#include "img/PSDFormat.h"
#include "core/ResourceHolder.h"
#include "core/Project.h"
#include "gui/ViaPoint.h"
#include "gui/res/res_Notifier.h"

namespace gui {

class ResourceTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    enum { kItemColumn = 0 };
    enum { kColumnCount = 1 };
    typedef QList<img::ResourceNode*> NodeList;

    ResourceTreeWidget(ViaPoint& aViaPoint, bool aUseCustomContext, QWidget* aParent);
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
    void endRenameEditor();
    void onItemSelectionChanged();
    void onContextMenuRequested(const QPoint& aPos);
    void onChangePathActionTriggered(bool aIsTriggered);
    void onRenameActionTriggered(bool aIsTriggered);
    void onReloadActionTriggered(bool aIsTriggered);
    void onDeleteActionTriggered(bool aIsTriggered);

    ViaPoint& mViaPoint;
    util::LinkPointer<core::Project> mProject;
    core::ResourceHolder* mHolder;
    QTreeWidgetItem* mActionItem;
    QAction* mChangePathAction;
    QAction* mRenameAction;
    QAction* mReloadAction;
    QAction* mDeleteAction;
};

} // namespace gui

#endif // GUI_RESOURCETREEWIDGET_H
