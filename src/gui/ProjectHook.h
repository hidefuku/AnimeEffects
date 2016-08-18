#ifndef GUI_PROJECTHOOK_H
#define GUI_PROJECTHOOK_H

#include <QTreeWidgetItem>
#include "core/Project.h"
#include "core/RenderInfo.h"

namespace gui
{

class ProjectHook : public core::Project::Hook
{
public:
    ProjectHook();

    void grabTreeRoot(QTreeWidgetItem* aRoot);
    QTreeWidgetItem* releaseTreeRoot();
    bool hasTreeRoot() const { return mTreeRoot; }

    core::RenderInfo& renderInfo() { return mRenderInfo; }
    const core::RenderInfo& renderInfo() const { return mRenderInfo; }

private:
    QScopedPointer<QTreeWidgetItem> mTreeRoot;
    core::RenderInfo mRenderInfo;
};

} // namespace gui

#endif // GUI_PROJECTHOOK_H
