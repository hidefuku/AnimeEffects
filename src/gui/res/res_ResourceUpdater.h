#ifndef GUI_RES_RESOURCEUPDATER_H
#define GUI_RES_RESOURCEUPDATER_H

#include "cmnd/Stack.h"
#include "img/PSDFormat.h"
#include "core/Project.h"
#include "gui/ViaPoint.h"
#include "gui/res/res_Item.h"
#include "gui/res/res_Notifier.h"

namespace gui {
namespace res {

class ResourceUpdater
{
public:
    ResourceUpdater(ViaPoint& aViaPoint, core::Project& aProject);
    void load(const QString& aFilePath);
    void reload(Item& aItem);

private:
    void reloadImages(
            cmnd::Stack& aStack,
            ModificationNotifier& aNotifier,
            const img::PSDFormat::Header& aHeader,
            img::ResourceNode& aCurNode,
            img::ResourceNode& aNewNode);

    bool tryReloadCorrespondingImages(
            const img::PSDFormat::Header& aHeader,
            QTreeWidgetItem& aTarget,
            img::ResourceNode* aNewTree);

    ViaPoint& mViaPoint;
    core::Project& mProject;
};

} // namespace res
} // namespace gui

#endif // GUI_RES_RESOURCEUPDATER_H
