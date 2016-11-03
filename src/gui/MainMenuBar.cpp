#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include "cmnd/BasicCommands.h"
#include "cmnd/ScopedMacro.h"
#include "core/ObjectNodeUtil.h"
#include "gui/MainMenuBar.h"
#include "gui/MainWindow.h"
#include "gui/ResourceDialog.h"
#include "gui/EasyDialog.h"
#include "gui/KeyBindingDialog.h"

namespace gui
{

MainMenuBar::MainMenuBar(MainWindow& aMainWindow, ViaPoint& aViaPoint, QWidget* aParent)
    : QMenuBar(aParent)
    , mViaPoint(aViaPoint)
    , mProject()
    , mProjectActions()
    , mShowResourceWindow()
{
    MainWindow* mainWindow = &aMainWindow;

    QMenu* fileMenu = new QMenu("File", this);
    {
        QAction* newProject   = new QAction("New Project...", this);
        QAction* openProject  = new QAction("Open Project...", this);
        QAction* saveProject  = new QAction("Save Project", this);
        QAction* closeProject = new QAction("Close Project", this);

        QMenu* exportAs = new QMenu("Export As", this);
        {
            QAction* pngs = new QAction("PNG Sequence...", this);
            QAction* ogv  = new QAction("Ogg Video...", this);
            connect(pngs, &QAction::triggered, mainWindow, &MainWindow::onExportPngSeqTriggered);
            connect(ogv, &QAction::triggered, mainWindow, &MainWindow::onExportVideoTriggered);
            exportAs->addAction(pngs);
            exportAs->addAction(ogv);
        }

        mProjectActions.push_back(saveProject);
        mProjectActions.push_back(closeProject);
        mProjectActions.push_back(exportAs->menuAction());

        connect(newProject, &QAction::triggered, mainWindow, &MainWindow::onNewProjectTriggered);
        connect(openProject, &QAction::triggered, mainWindow, &MainWindow::onOpenProjectTriggered);
        connect(saveProject, &QAction::triggered, mainWindow, &MainWindow::onSaveProjectTriggered);
        connect(closeProject, &QAction::triggered, mainWindow, &MainWindow::onCloseProjectTriggered);

        fileMenu->addAction(newProject);
        fileMenu->addAction(openProject);
        fileMenu->addSeparator();
        fileMenu->addAction(saveProject);
        fileMenu->addAction(exportAs->menuAction());
        fileMenu->addSeparator();
        fileMenu->addAction(closeProject);
    }

    QMenu* editMenu = new QMenu("Edit", this);
    {
        QAction* undo = new QAction("Undo", this);
        QAction* redo = new QAction("Redo", this);

        mProjectActions.push_back(undo);
        mProjectActions.push_back(redo);

        connect(undo, &QAction::triggered, mainWindow, &MainWindow::onUndoTriggered);
        connect(redo, &QAction::triggered, mainWindow, &MainWindow::onRedoTriggered);

        editMenu->addAction(undo);
        editMenu->addAction(redo);
    }

    QMenu* projMenu = new QMenu("Project", this);
    {
        QAction* canvSize = new QAction("Canvas Size...", this);
        QAction* maxFrame = new QAction("Max Frame...", this);
        QAction* loopAnim = new QAction("Loop...", this);

        mProjectActions.push_back(canvSize);
        mProjectActions.push_back(maxFrame);
        mProjectActions.push_back(loopAnim);

        connect(canvSize, &QAction::triggered, this, &MainMenuBar::onCanvasSizeTriggered);
        connect(maxFrame, &QAction::triggered, this, &MainMenuBar::onMaxFrameTriggered);
        connect(loopAnim, &QAction::triggered, this, &MainMenuBar::onLoopTriggered);

        projMenu->addAction(canvSize);
        projMenu->addAction(maxFrame);
        projMenu->addAction(loopAnim);
    }

    QMenu* windowMenu = new QMenu("Window", this);
    {
        QAction* resource = new QAction("Resource Window", this);
        resource->setCheckable(true);
        mShowResourceWindow = resource;

        connect(resource, &QAction::triggered, [&](bool aChecked)
        {
            if (aChecked)
            {
                aViaPoint.createResourceDialog();
            }
            else
            {
                if (aViaPoint.resourceDialog())
                {
                    aViaPoint.resourceDialog()->hide();
                }
            }
        });

        windowMenu->addAction(resource);
    }

    QMenu* optionMenu = new QMenu("Option", this);
    {
        QAction* keyBind = new QAction("Key Binding...", this);

        connect(keyBind, &QAction::triggered, [&](bool)
        {
            XC_PTR_ASSERT(aViaPoint.keyCommandMap());
            QScopedPointer<KeyBindingDialog> dialog(
                        new KeyBindingDialog(*aViaPoint.keyCommandMap(), this));
            dialog->exec();
        });

        optionMenu->addAction(keyBind);
    }

    QMenu* helpMenu = new QMenu("Help", this);
    {
        QAction* aboutMe = new QAction("About Anime Effects...", this);
        connect(aboutMe, &QAction::triggered, [=]()
        {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText("Anime Effects (Version 0.9.0)");

            QString infoText;
            infoText += "Build ABI : " + QSysInfo::buildAbi() + "\n";
            //infoText += "Build CPU : " + QSysInfo::buildCpuArchitecture() + "\n";
            //infoText += "Current CPU : " + QSysInfo::currentCpuArchitecture() + "\n";
            msgBox.setInformativeText(infoText);

            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
        });

        helpMenu->addAction(aboutMe);
    }

    this->addAction(fileMenu->menuAction());
    this->addAction(editMenu->menuAction());
    this->addAction(projMenu->menuAction());
    this->addAction(windowMenu->menuAction());
    this->addAction(optionMenu->menuAction());
    this->addAction(helpMenu->menuAction());

    // reset status
    setProject(nullptr);
}

void MainMenuBar::setProject(core::Project* aProject)
{
    mProject = aProject;

    if (mProject)
    {
        for (auto action : mProjectActions)
        {
            action->setEnabled(true);
        }
    }
    else
    {
        for (auto action : mProjectActions)
        {
            action->setEnabled(false);
        }
    }
}

void MainMenuBar::setShowResourceWindow(bool aShow)
{
    mShowResourceWindow->setChecked(aShow);
}

void MainMenuBar::onCanvasSizeTriggered()
{
    if (!mProject) return;

    // create dialog
    QScopedPointer<EasyDialog> dialog(new EasyDialog("Set Canvas Size", this));

    // create inner widgets
    QSpinBox* widthBox = nullptr;
    QSpinBox* heightBox = nullptr;
    auto curSize = mProject->attribute().imageSize();
    {
        auto devInfo = mViaPoint.glDeviceInfo();
        const int maxBufferSize = std::min(
                    devInfo.maxTextureSize,
                    devInfo.maxRenderBufferSize);
        XC_ASSERT(maxBufferSize > 0);

        auto form = new QFormLayout();
        form->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
        form->setLabelAlignment(Qt::AlignRight);

        auto sizeLayout = new QHBoxLayout();
        {
            widthBox = new QSpinBox();
            widthBox->setRange(1, maxBufferSize);
            widthBox->setValue(curSize.width());
            sizeLayout->addWidget(widthBox);

            heightBox = new QSpinBox();
            heightBox->setRange(1, maxBufferSize);
            heightBox->setValue(curSize.height());
            sizeLayout->addWidget(heightBox);
        }
        form->addRow("size :", sizeLayout);

#if 0
        auto origin = new QGridLayout();
        {
            for (int y = 0; y < 3; ++y)
            {
                for (int x = 0; x < 3; ++x)
                {
                    auto button = new QPushButton();
                    button->setFixedSize(QSize(24, 24));
                    //button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                    //button->setContentsMargins(0, 0, 0, 0);
                    origin->addWidget(button, x, y, Qt::AlignCenter);
                }
            }

            origin->setContentsMargins(0, 0, 0, 0);
            origin->setSpacing(0);
            origin->setSizeConstraint(QLayout::SetFixedSize);

            /*
            origin->setMargin(0);
            for (int i = 0; i < 3; ++i)
            {
                origin->setRowMinimumHeight(i, 1);
                origin->setColumnMinimumWidth(i, 1);
            }
            */

            auto originWrap = new QGroupBox();
            originWrap->setLayout(origin);
            form->addRow("origin : ", originWrap);
        }
#endif

        auto group = new QGroupBox("Parameter");
        group->setLayout(form);
        dialog->setMainWidget(group);
    }
    dialog->setOkCancel();
    dialog->fixSize();

    // execute dialog
    dialog->exec();
    if (dialog->result() != QDialog::Accepted) return;

    // get new canvas size
    const QSize newSize(widthBox->value(), heightBox->value());
    XC_ASSERT(!newSize.isEmpty());
    if (curSize == newSize) return;

    // create commands
    {
        cmnd::ScopedMacro macro(mProject->commandStack(), "change canvas size");

        core::Project* projectPtr = mProject;
        auto command = new cmnd::Delegatable([=]()
        {
            projectPtr->attribute().setImageSize(newSize);
            auto event = core::ProjectEvent::imageSizeChangeEvent(*projectPtr);
            projectPtr->onProjectAttributeModified(event, false);
            this->onProjectAttributeUpdated();
            this->onVisualUpdated();
        },
        [=]()
        {
            projectPtr->attribute().setImageSize(curSize);
            auto event = core::ProjectEvent::imageSizeChangeEvent(*projectPtr);
            projectPtr->onProjectAttributeModified(event, true);
            this->onProjectAttributeUpdated();
            this->onVisualUpdated();
        });
        mProject->commandStack().push(command);
    }
}

void MainMenuBar::onMaxFrameTriggered()
{
    if (!mProject) return;

    // create dialog
    QScopedPointer<EasyDialog> dialog(new EasyDialog("Set Max Frame", this));

    // create inner widgets
    QSpinBox* maxFrameBox = nullptr;
    auto curMaxFrame = mProject->attribute().maxFrame();
    {
        auto form = new QFormLayout();
        form->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
        form->setLabelAlignment(Qt::AlignRight);

        auto layout = new QHBoxLayout();
        {
            maxFrameBox = new QSpinBox();
            maxFrameBox->setRange(1, std::numeric_limits<int>::max());
            maxFrameBox->setValue(curMaxFrame);
            layout->addWidget(maxFrameBox);
        }
        form->addRow("max frame :", layout);

        auto group = new QGroupBox("Parameter");
        group->setLayout(form);
        dialog->setMainWidget(group);
    }

    dialog->setOkCancel([=](int aIndex)->bool
    {
        if (aIndex == 0)
        {
            return this->confirmMaxFrameUpdating(maxFrameBox->value());
        }
        return true;
    });
    dialog->fixSize();

    // execute dialog
    dialog->exec();
    if (dialog->result() != QDialog::Accepted) return;

    // get new canvas size
    const int newMaxFrame = maxFrameBox->value();
    XC_ASSERT(newMaxFrame > 0);
    if (curMaxFrame == newMaxFrame) return;

    // create commands
    {
        cmnd::ScopedMacro macro(mProject->commandStack(), "change max frame");

        core::Project* projectPtr = mProject;
        auto command = new cmnd::Delegatable([=]()
        {
            projectPtr->attribute().setMaxFrame(newMaxFrame);
            auto event = core::ProjectEvent::maxFrameChangeEvent(*projectPtr);
            projectPtr->onProjectAttributeModified(event, false);
            this->onProjectAttributeUpdated();
            this->onVisualUpdated();
        },
        [=]()
        {
            projectPtr->attribute().setMaxFrame(curMaxFrame);
            auto event = core::ProjectEvent::maxFrameChangeEvent(*projectPtr);
            projectPtr->onProjectAttributeModified(event, true);
            this->onProjectAttributeUpdated();
            this->onVisualUpdated();
        });
        mProject->commandStack().push(command);
    }
}

bool MainMenuBar::confirmMaxFrameUpdating(int aNewMaxFrame) const
{
    XC_ASSERT(aNewMaxFrame > 0);
    if (!mProject) return false;

    auto curMaxFrame = mProject->attribute().maxFrame();
    if (curMaxFrame <= aNewMaxFrame) return true;

    if (!core::ObjectNodeUtil::thereAreSomeKeysExceedingFrame(
                mProject->objectTree().topNode(), aNewMaxFrame))
    {
        return true;
    }

    QMessageBox msgBox;
    msgBox.setText("Can not set the specified frame.");
    msgBox.setInformativeText("There are some keys that exeeding the specified frame.");
    msgBox.exec();

    return false;
}

void MainMenuBar::onLoopTriggered()
{
    if (!mProject) return;

    // create dialog
    QScopedPointer<EasyDialog> dialog(new EasyDialog("Set Animation Loop", this));

    // create inner widgets
    QCheckBox* loopBox = nullptr;
    auto curLoop = mProject->attribute().loop();
    {
        auto form = new QFormLayout();
        form->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
        form->setLabelAlignment(Qt::AlignRight);

        auto layout = new QHBoxLayout();
        {
            loopBox = new QCheckBox();
            loopBox->setChecked(curLoop);
            layout->addWidget(loopBox);
        }
        form->addRow("loop animation :", layout);

        auto group = new QGroupBox("Parameter");
        group->setLayout(form);
        dialog->setMainWidget(group);
    }

    dialog->setOkCancel();
    dialog->fixSize();

    // execute dialog
    dialog->exec();
    if (dialog->result() != QDialog::Accepted) return;

    // get new loop setting
    const bool newLoop = loopBox->isChecked();
    if (curLoop == newLoop) return;

    // create commands
    {
        cmnd::ScopedMacro macro(mProject->commandStack(), "change animation loop setting");

        core::Project* projectPtr = mProject;
        auto command = new cmnd::Delegatable([=]()
        {
            projectPtr->attribute().setLoop(newLoop);
            auto event = core::ProjectEvent::loopChangeEvent(*projectPtr);
            projectPtr->onProjectAttributeModified(event, false);
            this->onProjectAttributeUpdated();
            this->onVisualUpdated();
        },
        [=]()
        {
            projectPtr->attribute().setLoop(curLoop);
            auto event = core::ProjectEvent::loopChangeEvent(*projectPtr);
            projectPtr->onProjectAttributeModified(event, true);
            this->onProjectAttributeUpdated();
            this->onVisualUpdated();
        });
        mProject->commandStack().push(command);
    }
}

} // namespace gui
