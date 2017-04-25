#include <QFile>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDomDocument>
#include "cmnd/BasicCommands.h"
#include "cmnd/ScopedMacro.h"
#include "core/ObjectNodeUtil.h"
#include "ctrl/CmndName.h"
#include "gui/MainMenuBar.h"
#include "gui/MainWindow.h"
#include "gui/ResourceDialog.h"
#include "gui/EasyDialog.h"
#include "gui/KeyBindingDialog.h"
#include "gui/GeneralSettingDialog.h"
#include "gui/MouseSettingDialog.h"

namespace gui
{
//-------------------------------------------------------------------------------------------------
QDomDocument getVideoExportDocument()
{
    QFile file("./data/encode/VideoEncode.txt");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << file.errorString();
        return QDomDocument();
    }

    QDomDocument prop;
    QString errorMessage;
    int errorLine = 0;
    int errorColumn = 0;
    if (!prop.setContent(&file, false, &errorMessage, &errorLine, &errorColumn))
    {
        qDebug() << "invalid xml file. "
                 << file.fileName()
                 << errorMessage << ", line = " << errorLine
                 << ", column = " << errorColumn;
        return QDomDocument();
    }
    file.close();

    return prop;
}

//-------------------------------------------------------------------------------------------------
MainMenuBar::MainMenuBar(MainWindow& aMainWindow, ViaPoint& aViaPoint, QWidget* aParent)
    : QMenuBar(aParent)
    , mViaPoint(aViaPoint)
    , mProject()
    , mProjectActions()
    , mShowResourceWindow()
    , mVideoFormats()
{
    // load the list of video formats from a setting file.
    loadVideoFormats();

    MainWindow* mainWindow = &aMainWindow;

    QMenu* fileMenu = new QMenu(tr("File"), this);
    {
        QAction* newProject     = new QAction(tr("New Project..."), this);
        QAction* openProject    = new QAction(tr("Open Project..."), this);
        QAction* saveProject    = new QAction(tr("Save Project"), this);
        QAction* saveProjectAs  = new QAction(tr("Save Project As..."), this);
        QAction* closeProject   = new QAction(tr("Close Project"), this);

        QMenu* exportAs = new QMenu(tr("Export As"), this);
        {
            ctrl::VideoFormat gifFormat;
            gifFormat.name = "gif";
            gifFormat.label = "GIF";
            gifFormat.icodec = "ppm";

            QAction* jpgs = new QAction(tr("JPEG Sequence..."), this);
            QAction* pngs = new QAction(tr("PNG Sequence..."), this);
            QAction* gif  = new QAction(tr("GIF Animation..."), this);
            connect(jpgs, &QAction::triggered, [=](){ mainWindow->onExportImageSeqTriggered("jpg"); });
            connect(pngs, &QAction::triggered, [=](){ mainWindow->onExportImageSeqTriggered("png"); });
            connect(gif,  &QAction::triggered, [=](){ mainWindow->onExportVideoTriggered(gifFormat); });
            exportAs->addAction(jpgs);
            exportAs->addAction(pngs);
            exportAs->addAction(gif);

            for (auto format : mVideoFormats)
            {
                QAction* video = new QAction(format.label + " " + tr("Video") + "...", this);
                connect(video,  &QAction::triggered, [=](){ mainWindow->onExportVideoTriggered(format); });
                exportAs->addAction(video);
            }
        }

        mProjectActions.push_back(saveProject);
        mProjectActions.push_back(saveProjectAs);
        mProjectActions.push_back(closeProject);
        mProjectActions.push_back(exportAs->menuAction());

        connect(newProject, &QAction::triggered, mainWindow, &MainWindow::onNewProjectTriggered);
        connect(openProject, &QAction::triggered, mainWindow, &MainWindow::onOpenProjectTriggered);
        connect(saveProject, &QAction::triggered, mainWindow, &MainWindow::onSaveProjectTriggered);
        connect(saveProjectAs, &QAction::triggered, mainWindow, &MainWindow::onSaveProjectAsTriggered);
        connect(closeProject, &QAction::triggered, mainWindow, &MainWindow::onCloseProjectTriggered);

        fileMenu->addAction(newProject);
        fileMenu->addAction(openProject);
        fileMenu->addSeparator();
        fileMenu->addAction(saveProject);
        fileMenu->addAction(saveProjectAs);
        fileMenu->addAction(exportAs->menuAction());
        fileMenu->addSeparator();
        fileMenu->addAction(closeProject);
    }

    QMenu* editMenu = new QMenu(tr("Edit"), this);
    {
        QAction* undo = new QAction(tr("Undo"), this);
        QAction* redo = new QAction(tr("Redo"), this);

        mProjectActions.push_back(undo);
        mProjectActions.push_back(redo);

        connect(undo, &QAction::triggered, mainWindow, &MainWindow::onUndoTriggered);
        connect(redo, &QAction::triggered, mainWindow, &MainWindow::onRedoTriggered);

        editMenu->addAction(undo);
        editMenu->addAction(redo);
    }

    QMenu* projMenu = new QMenu(tr("Project"), this);
    {
        QAction* canvSize = new QAction(tr("Canvas Size..."), this);
        QAction* maxFrame = new QAction(tr("Max Frame..."), this);
        QAction* loopAnim = new QAction(tr("Loop..."), this);

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

    QMenu* windowMenu = new QMenu(tr("Window"), this);
    {
        QAction* resource = new QAction(tr("Resource Window"), this);
        resource->setCheckable(true);
        mShowResourceWindow = resource;

        connect(resource, &QAction::triggered, [&](bool aChecked)
        {
            if (aViaPoint.resourceDialog())
            {
                aViaPoint.resourceDialog()->setVisible(aChecked);
            }
        });

        windowMenu->addAction(resource);
    }

    QMenu* optionMenu = new QMenu(tr("Option"), this);
    {
        QAction* general = new QAction(tr("General Settings..."), this);
        QAction* mouse   = new QAction(tr("Mouse Settings..."), this);
        QAction* keyBind = new QAction(tr("Key Bindings..."), this);

        connect(general, &QAction::triggered, [&](bool)
        {
            QScopedPointer<GeneralSettingDialog> dialog(
                        new GeneralSettingDialog(this));
            dialog->exec();
        });

        connect(mouse, &QAction::triggered, [&](bool)
        {
            QScopedPointer<MouseSettingDialog> dialog(
                        new MouseSettingDialog(aViaPoint, this));
            dialog->exec();
        });

        connect(keyBind, &QAction::triggered, [&](bool)
        {
            XC_PTR_ASSERT(aViaPoint.keyCommandMap());
            QScopedPointer<KeyBindingDialog> dialog(
                        new KeyBindingDialog(*aViaPoint.keyCommandMap(), this));
            dialog->exec();
        });

        optionMenu->addAction(general);
        optionMenu->addAction(mouse);
        optionMenu->addAction(keyBind);
    }

    QMenu* helpMenu = new QMenu(tr("Help"), this);
    {
        QAction* aboutMe = new QAction(tr("About AnimeEffects..."), this);
        connect(aboutMe, &QAction::triggered, [=]()
        {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Information);
            auto versionString = QString::number(AE_MAJOR_VERSION) + "." + QString::number(AE_MINOR_VERSION) + "." + QString::number(AE_MICRO_VERSION);
            auto formatVersionString = QString::number(AE_PROJECT_FORMAT_MAJOR_VERSION) + "." + QString::number(AE_PROJECT_FORMAT_MINOR_VERSION);
            auto platform = QSysInfo::productType();
            msgBox.setText(QString("AnimeEffects for ") + platform + " version " + versionString);

            QString detail;
            detail += "Version: " + versionString + "\n";
            detail += "Platform: " + platform + " " + QSysInfo::productVersion() + "\n";
            detail += "Build ABI: " + QSysInfo::buildAbi() + "\n";
            detail += "Build CPU: " + QSysInfo::buildCpuArchitecture() + "\n";
            detail += "Current CPU: " + QSysInfo::currentCpuArchitecture() + "\n";
            detail += "Current GPU: " + QString(this->mViaPoint.glDeviceInfo().renderer.c_str()) + "\n";
            detail += "GPU Vender: " + QString(this->mViaPoint.glDeviceInfo().vender.c_str()) + "\n";
            detail += "OpenGL Version: " + QString(this->mViaPoint.glDeviceInfo().version.c_str()) + "\n";
            detail += "Format Version: " + formatVersionString + "\n";
            msgBox.setDetailedText(detail);

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

void MainMenuBar::loadVideoFormats()
{
    QDomDocument doc = getVideoExportDocument();
    QDomElement domRoot = doc.firstChildElement("video_encode");

    // for each format
    QDomElement domFormat = domRoot.firstChildElement("format");
    while (!domFormat.isNull())
    {
        ctrl::VideoFormat format;
        // neccessary attribute
        format.name = domFormat.attribute("name");
        if (format.name.isEmpty()) continue;
        // optional attributes
        format.label = domFormat.attribute("label");
        format.icodec = domFormat.attribute("icodec");
        format.command = domFormat.attribute("command");
        if (format.label.isEmpty()) format.label = format.name;
        if (format.icodec.isEmpty()) format.icodec = "png";
        // add one format
        mVideoFormats.push_back(format);

        // for each codec
        QDomElement domCodec = domFormat.firstChildElement("codec");
        while (!domCodec.isNull())
        {
            ctrl::VideoCodec codec;
            // neccessary attribute
            codec.name = domCodec.attribute("name");
            if (codec.name.isEmpty()) continue;
            // optional attributes
            codec.label = domCodec.attribute("label");
            codec.icodec = domCodec.attribute("icodec");
            codec.command = domCodec.attribute("command");
            if (codec.label.isEmpty()) codec.label = codec.name;
            if (codec.icodec.isEmpty()) codec.icodec = format.icodec;
            if (codec.command.isEmpty()) codec.command = format.command;
            {
                auto hints = domCodec.attribute("hint").split(',');
                for (QString hint : hints)
                {
                    hint = hint.trimmed();
                    if (hint == "lossless") codec.lossless = true;
                    else if (hint == "transparent") codec.transparent = true;
                    else if (hint == "colorspace") codec.colorspace = true;
                }
            }
            // add one codec
            mVideoFormats.back().codecs.push_back(codec);

            // to next sibling
            domCodec = domCodec.nextSiblingElement("codec");
        }
        // to next sibling
        domFormat = domFormat.nextSiblingElement("format");
    }
}

//-------------------------------------------------------------------------------------------------
ProjectCanvasSizeSettingDialog::ProjectCanvasSizeSettingDialog(
        ViaPoint& aViaPoint, core::Project& aProject, QWidget *aParent)
    : EasyDialog(tr("Set Canvas Size"), aParent)
    , mViaPoint(aViaPoint)
    , mProject(aProject)
{
    // create inner widgets
    auto curSize = mProject.attribute().imageSize();
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
            mWidthBox = new QSpinBox();
            mWidthBox->setRange(1, maxBufferSize);
            mWidthBox->setValue(curSize.width());
            sizeLayout->addWidget(mWidthBox);

            mHeightBox = new QSpinBox();
            mHeightBox->setRange(1, maxBufferSize);
            mHeightBox->setValue(curSize.height());
            sizeLayout->addWidget(mHeightBox);
        }
        form->addRow(tr("size :"), sizeLayout);

        auto group = new QGroupBox(tr("Parameters"));
        group->setLayout(form);
        this->setMainWidget(group);
    }
    this->setOkCancel();
    this->fixSize();
}

void MainMenuBar::onCanvasSizeTriggered()
{
    if (!mProject) return;

    auto curSize = mProject->attribute().imageSize();

    // create dialog
    QScopedPointer<ProjectCanvasSizeSettingDialog> dialog(
                new ProjectCanvasSizeSettingDialog(mViaPoint, *mProject, this));

    // execute dialog
    dialog->exec();
    if (dialog->result() != QDialog::Accepted) return;

    // get new canvas size
    const QSize newSize = dialog->canvasSize();
    XC_ASSERT(!newSize.isEmpty());
    if (curSize == newSize) return;

    // create commands
    {
        cmnd::ScopedMacro macro(mProject->commandStack(),
                                CmndName::tr("change the canvas size"));

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

//-------------------------------------------------------------------------------------------------
ProjectMaxFrameSettingDialog::ProjectMaxFrameSettingDialog(core::Project& aProject, QWidget *aParent)
    : EasyDialog(tr("Set Max Frame"), aParent)
    , mProject(aProject)
    , mMaxFrameBox()
{
    // create inner widgets
    auto curMaxFrame = mProject.attribute().maxFrame();
    {
        auto form = new QFormLayout();
        form->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
        form->setLabelAlignment(Qt::AlignRight);

        auto layout = new QHBoxLayout();
        {
            mMaxFrameBox = new QSpinBox();
            mMaxFrameBox->setRange(1, std::numeric_limits<int>::max());
            mMaxFrameBox->setValue(curMaxFrame);
            layout->addWidget(mMaxFrameBox);
        }
        form->addRow(tr("max frame :"), layout);

        auto group = new QGroupBox(tr("Parameters"));
        group->setLayout(form);
        this->setMainWidget(group);
    }

    this->setOkCancel([=](int aIndex)->bool
    {
        if (aIndex == 0)
        {
            return this->confirmMaxFrameUpdating(this->mMaxFrameBox->value());
        }
        return true;
    });
    this->fixSize();
}

bool ProjectMaxFrameSettingDialog::confirmMaxFrameUpdating(int aNewMaxFrame) const
{
    XC_ASSERT(aNewMaxFrame > 0);

    auto curMaxFrame = mProject.attribute().maxFrame();
    if (curMaxFrame <= aNewMaxFrame) return true;

    if (!core::ObjectNodeUtil::thereAreSomeKeysExceedingFrame(
                mProject.objectTree().topNode(), aNewMaxFrame))
    {
        return true;
    }

    auto message1 = tr("Can not set the specified frame.");
    auto message2 = tr("There are some keys that exeeding the specified frame.");
    QMessageBox::warning(nullptr, tr("Operation Error"), message1 + "\n" + message2);
    return false;
}

void MainMenuBar::onMaxFrameTriggered()
{
    if (!mProject) return;

    auto curMaxFrame = mProject->attribute().maxFrame();

    // create dialog
    QScopedPointer<ProjectMaxFrameSettingDialog> dialog(
                new ProjectMaxFrameSettingDialog(*mProject, this));

    // execute dialog
    dialog->exec();
    if (dialog->result() != QDialog::Accepted) return;

    // get new canvas size
    const int newMaxFrame = dialog->maxFrame();
    XC_ASSERT(newMaxFrame > 0);
    if (curMaxFrame == newMaxFrame) return;

    // create commands
    {
        cmnd::ScopedMacro macro(mProject->commandStack(),
                                CmndName::tr("change the max frame"));

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

//-------------------------------------------------------------------------------------------------
ProjectLoopSettingDialog::ProjectLoopSettingDialog(core::Project& aProject, QWidget* aParent)
    : EasyDialog(tr("Set Animation Loop"), aParent)
{
    // create inner widgets
    auto curLoop = aProject.attribute().loop();
    {
        auto form = new QFormLayout();
        form->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
        form->setLabelAlignment(Qt::AlignRight);

        auto layout = new QHBoxLayout();
        {
            mLoopBox = new QCheckBox();
            mLoopBox->setChecked(curLoop);
            layout->addWidget(mLoopBox);
        }
        form->addRow(tr("loop animation :"), layout);

        auto group = new QGroupBox(tr("Parameters"));
        group->setLayout(form);
        this->setMainWidget(group);
    }

    this->setOkCancel();
    this->fixSize();
}

void MainMenuBar::onLoopTriggered()
{
    if (!mProject) return;

    auto curLoop = mProject->attribute().loop();

    // create dialog
    QScopedPointer<ProjectLoopSettingDialog> dialog(new ProjectLoopSettingDialog(*mProject, this));

    // execute dialog
    dialog->exec();
    if (dialog->result() != QDialog::Accepted) return;

    // get new loop setting
    const bool newLoop = dialog->isCheckedLoopBox();
    if (curLoop == newLoop) return;

    // create commands
    {
        cmnd::ScopedMacro macro(mProject->commandStack(),
                                CmndName::tr("change the animation loop setting"));

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
