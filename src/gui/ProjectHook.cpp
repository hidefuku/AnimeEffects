#include "gui/ProjectHook.h"

namespace gui
{

ProjectHook::ProjectHook()
    : mObjectTree()
    , mResourceTrees()
    , mRenderInfo()
{
}

ProjectHook::~ProjectHook()
{
    deleteResourceTrees();
}

void ProjectHook::grabTreeRoot(QTreeWidgetItem* aRoot)
{
    mObjectTree.reset(aRoot);
}

QTreeWidgetItem* ProjectHook::releaseTreeRoot()
{
    return mObjectTree.take();
}

void ProjectHook::grabResourceTrees(QVector<QTreeWidgetItem*>* aTrees)
{
    deleteResourceTrees();
    mResourceTrees = aTrees;
}

QVector<QTreeWidgetItem*>* ProjectHook::releaseResourceTrees()
{
    auto trees = mResourceTrees;
    mResourceTrees = nullptr;
    return trees;
}

void ProjectHook::deleteResourceTrees()
{
    if (mResourceTrees)
    {
        for (auto tree : *mResourceTrees)
        {
            delete tree;
        }
        delete mResourceTrees;
        mResourceTrees = nullptr;
    }
}

} // namespace gui

