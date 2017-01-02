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
    virtual ~ProjectHook();

    void grabObjectTrees(QVector<QTreeWidgetItem*>* aTrees);
    QVector<QTreeWidgetItem*>* releaseObjectTrees();
    bool hasObjectTrees() const { return mObjectTrees; }

    void grabResourceTrees(QVector<QTreeWidgetItem*>* aTrees);
    QVector<QTreeWidgetItem*>* releaseResourceTrees();
    bool hasResourceTrees() const { return mResourceTrees; }

    core::RenderInfo& renderInfo() { return mRenderInfo; }
    const core::RenderInfo& renderInfo() const { return mRenderInfo; }

private:
    void deleteObjectTrees();
    void deleteResourceTrees();

    QVector<QTreeWidgetItem*>* mObjectTrees;
    QVector<QTreeWidgetItem*>* mResourceTrees;
    core::RenderInfo mRenderInfo;
};

} // namespace gui

#endif // GUI_PROJECTHOOK_H
