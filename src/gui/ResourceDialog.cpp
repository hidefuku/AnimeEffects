#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include "img/PSDReader.h"
#include "img/PSDUtil.h"
#include "img/Util.h"
#include "gui/ResourceDialog.h"
#include "gui/MainMenuBar.h"
#include "gui/res/res_ResourceUpdater.h"

namespace gui
{

ResourceDialog::ResourceDialog(ViaPoint& aViaPoint, bool aModal, QWidget* aParent)
    : EasyDialog("Resource Dialog", aParent, aModal)
    , mViaPoint(aViaPoint)
    , mProject()
    , mTree()
{
    // menu bar
    if (!aModal)
    {
        auto menuBar = new QMenuBar(this);
        auto fileMenu = new QMenu("File", menuBar);
        auto addResource = new QAction("Add Resource", fileMenu);
        connect(addResource, &QAction::triggered, this, &ResourceDialog::onAddResourceTriggered);
        fileMenu->addAction(addResource);
        menuBar->addMenu(fileMenu);
        menuBar->show();
        this->setMenuBar(menuBar);

        connect(this, &QDialog::finished, [&](int)
        {
            if (aViaPoint.mainMenuBar())
            {
                aViaPoint.mainMenuBar()->setShowResourceWindow(false);
            }
        });
    }

    // resource tree
    mTree = new res::ResourceTree(aViaPoint, !aModal, this);
    this->setMainWidget(mTree, false);

    // modal only
    if (aModal)
    {
        // button box
        this->setOkCancel();
        this->setOkEnable(false);

        // updater
        mTree->onNodeSelectionChanged.connect([=](const NodeList& aNodes)
        {
            this->mNodeList.clear();
            for (auto node : aNodes)
            {
                if (node->hasImage())
                {
                    this->mNodeList.push_back(node);
                }
            }
            const bool isValid = !this->mNodeList.isEmpty();
            this->setOkEnable(isValid);
        });
    }
}

void ResourceDialog::setProject(core::Project* aProject)
{
    mTree->resetTreeView();
    if (aProject)
    {
        mProject = aProject->pointee();
    }
    else
    {
        mProject.reset();
    }
    mTree->setProject(aProject);
}

void ResourceDialog::updateResources()
{
    if (mProject)
    {
        mTree->resetTreeView(mProject->resourceHolder());
    }
}

void ResourceDialog::updateResources(const util::TreePos& aRoot)
{
    if (mProject)
    {
        mTree->resetTreeView(mProject->resourceHolder(), aRoot);
    }
}

void ResourceDialog::updateResourcePath()
{
    if (mProject)
    {
        mTree->updateTreeRootName(mProject->resourceHolder());
    }
}

bool ResourceDialog::hasValidNode() const
{
    return this->result() == QDialog::Accepted && !mNodeList.isEmpty();
}

void ResourceDialog::onAddResourceTriggered(bool)
{
    if (!mProject) return;

    const QString fileName = QFileDialog::getOpenFileName(
                this, "Open File", "", "ImageFile (*.psd)");
    if (fileName.isEmpty()) return;

    res::ResourceUpdater updater(mViaPoint, *mProject);
    updater.load(fileName);
}

} // namespace gui

