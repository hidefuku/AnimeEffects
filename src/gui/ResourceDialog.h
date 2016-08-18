#ifndef GUI_RESOURCEDIALOG_H
#define GUI_RESOURCEDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include "util/LinkPointer.h"
#include "core/Project.h"
#include "gui/EasyDialog.h"
#include "gui/ResourceDialog.h"
#include "gui/res/res_ResourceTree.h"

namespace gui
{

class ResourceDialog : public EasyDialog
{
public:
    typedef QList<img::ResourceNode*> NodeList;
    ResourceDialog(ViaPoint& aViaPoint, bool aModal, QWidget* aParent);

    void setProject(core::Project* aProject);
    void updateResources();
    void updateResources(const util::TreePos& aRoot);
    void updateResourcePath();

    bool hasValidNode() const;
    const NodeList& nodeList() const { return mNodeList; }

private:
    void onAddResourceTriggered(bool);
    img::ResourceNode* createResourceNodes(img::PSDFormat& aFormat);

    ViaPoint& mViaPoint;
    util::LinkPointer<core::Project> mProject;
    res::ResourceTree* mTree;
    NodeList mNodeList;
};

} // namespace gui

#endif // GUI_RESOURCEDIALOG_H
