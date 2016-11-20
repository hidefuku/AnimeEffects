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
    void remove(Item& aItem);

private:
    img::ResourceNode* createResourceTree(const QString& aFilePath, bool aLoadImage);
    img::ResourceNode* createQImageTree(const QString& aFilePath, bool aLoadImage) const;
    img::ResourceNode* createPsdTree(const QString& aFilePath, bool aLoadImage);

    void createImageReloaderRecursive(
            cmnd::Stack& aStack,
            ModificationNotifier& aNotifier,
            img::ResourceNode& aCurNode,
            img::ResourceNode& aNewNode);

    void createAbandonedImageRemoverRecursive(
            cmnd::Stack& aStack, img::ResourceNode& aNode);

    bool tryReloadCorrespondingImages(
            QTreeWidgetItem& aTarget,
            img::ResourceNode* aNewTree);

    ViaPoint& mViaPoint;
    core::Project& mProject;
    std::unique_ptr<img::PSDFormat> mPSDFormat;
};

} // namespace res
} // namespace gui

#endif // GUI_RES_RESOURCEUPDATER_H
