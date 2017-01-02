#include "gui/ProjectHook.h"

namespace gui
{

ProjectHook::ProjectHook()
    : mObjectTrees()
    , mResourceTrees()
    , mRenderInfo()
{
}

ProjectHook::~ProjectHook()
{
    deleteObjectTrees();
    deleteResourceTrees();
}

void ProjectHook::grabObjectTrees(QVector<QTreeWidgetItem*>* aTrees)
{
    deleteObjectTrees();
    mObjectTrees = aTrees;
}

QVector<QTreeWidgetItem*>* ProjectHook::releaseObjectTrees()
{
    auto trees = mObjectTrees;
    mObjectTrees = nullptr;
    return trees;
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

void ProjectHook::deleteObjectTrees()
{
    if (mObjectTrees)
    {
        for (auto tree : *mObjectTrees)
        {
            delete tree;
        }
        delete mObjectTrees;
        mObjectTrees = nullptr;
    }
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

