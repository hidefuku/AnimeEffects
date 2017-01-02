#ifndef GUI_RESOURCEDIALOG_H
#define GUI_RESOURCEDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include "util/LinkPointer.h"
#include "core/Project.h"
#include "gui/EasyDialog.h"
#include "gui/ResourceDialog.h"
#include "gui/ResourceTreeWidget.h"

namespace gui
{

class ResourceDialog : public EasyDialog
{
    Q_OBJECT
public:
    typedef QList<img::ResourceNode*> NodeList;
    ResourceDialog(ViaPoint& aViaPoint, bool aModal, QWidget* aParent);

    void setProject(core::Project* aProject);
    void updateResourcePath();

    bool hasValidNode() const;
    const NodeList& nodeList() const { return mNodeList; }

private:
    virtual void keyPressEvent(QKeyEvent* aEvent);
    virtual void keyReleaseEvent(QKeyEvent* aEvent);

    void onAddResourceTriggered(bool);

    ViaPoint& mViaPoint;
    util::LinkPointer<core::Project> mProject;
    ResourceTreeWidget* mTree;
    NodeList mNodeList;
};

} // namespace gui

#endif // GUI_RESOURCEDIALOG_H
