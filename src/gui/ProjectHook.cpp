#include "gui/ProjectHook.h"

namespace gui
{

ProjectHook::ProjectHook()
    : mTreeRoot()
    , mRenderInfo()
{
}

void ProjectHook::grabTreeRoot(QTreeWidgetItem* aRoot)
{
    mTreeRoot.reset(aRoot);
}

QTreeWidgetItem* ProjectHook::releaseTreeRoot()
{
    return mTreeRoot.take();
}

} // namespace gui

