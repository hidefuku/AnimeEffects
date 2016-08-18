#include <QFileDialog>
#include <QDockWidget>
#include <QGraphicsDropShadowEffect>
#include <QShortcut>
#include <QElapsedTimer>
#include <QMessageBox>
#include "util/IProgressReporter.h"
#include "ctrl/Exporter.h"
#include "gui/MainWindow.h"
#include "gui/ExportDialog.h"
#include "gui/NewProjectDialog.h"
#include "gui/ResourceDialog.h"
#include "gui/ProjectHook.h"
#include "gui/menu/menu_ProgressReporter.h"

namespace
{
class EventSuspender
{
    QWriteLocker mRenderingLocker;
public:
    EventSuspender(gui::MainDisplayWidget& aMainDisplay, gui::TargetWidget& aTarget)
        : mRenderingLocker(&aMainDisplay.renderingLock())
    {
        // stop animation
        aTarget.stop();
    }
};

}

namespace gui
{

MainWindow::MainWindow(
        QWidget* aParent, ctrl::System* aSystem, GUIResourceSet* aResources)
    : QMainWindow(aParent)
    , mSystem(aSystem)
    , mResourceSet(aResources)
    , mViaPoint(this)
    , mMainMenuBar()
    , mMainDisplay()
    , mProjectTabBar()
    , mTarget()
    , mProperty()
    , mTool()
    , mDriverHolder()
    , mCurrent()
{
    // setup default opengl format
    {
        QSurfaceFormat format;
        format.setSamples(4);
        QSurfaceFormat::setDefaultFormat(format);
    }

    // setup UI
    {
        this->setObjectName(QStringLiteral("MainWindow"));
        this->resize(1280, 800);

        QFile stylesheet("data/stylesheet/standard.ssa");
        if (stylesheet.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            this->setStyleSheet(QTextStream(&stylesheet).readAll());
        }

        this->setMouseTracking(true);
        this->setFocusPolicy(Qt::NoFocus);
        this->setAcceptDrops(false);
        this->setTabShape(QTabWidget::Rounded);
        this->setDockOptions(QMainWindow::AnimatedDocks);
        this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    }

    {
        mMainMenuBar = new MainMenuBar(*this, mViaPoint, this);
        mViaPoint.setMainMenuBar(mMainMenuBar);
        this->setMenuBar(mMainMenuBar);
    }

    {
        mMainDisplay = new MainDisplayWidget(mViaPoint, this);
        this->setCentralWidget(mMainDisplay);

        mProjectTabBar = new ProjectTabBar(mMainDisplay);
        mMainDisplay->setProjectTabBar(mProjectTabBar);
        mProjectTabBar->onCurrentChanged.connect(this, &MainWindow::onProjectTabChanged);
    }

    {
        QDockWidget* dockWidget = new QDockWidget(this);
        dockWidget->setWindowTitle("Target Widget");
        dockWidget->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
        dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
        this->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

        mTarget = new TargetWidget(mViaPoint, *mResourceSet, dockWidget, QSize(256, 256));
        dockWidget->setWidget(mTarget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget(this);
        dockWidget->setWindowTitle("Property Widget");
        dockWidget->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        this->addDockWidget(Qt::RightDockWidgetArea, dockWidget);

        QFile stylesheet("data/stylesheet/propertywidget.ssa");
        if (stylesheet.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            dockWidget->setStyleSheet(QTextStream(&stylesheet).readAll());
        }

#if 0
        mProperty = new PropertyWidget(dockWidget);
        dockWidget->setWidget(mProperty);
#else
        auto splitter = new QSplitter(Qt::Vertical, dockWidget);
        dockWidget->setWidget(splitter);

        mProperty = new PropertyWidget(splitter);
        splitter->addWidget(mProperty);

        auto textEdit = new QPlainTextEdit(splitter);
        textEdit->setUndoRedoEnabled(false);
        textEdit->setMaximumBlockCount(32);
        textEdit->setReadOnly(true);
        mViaPoint.setLogView(textEdit);

        splitter->addWidget(textEdit);

        splitter->setCollapsible(0, false);
        splitter->setCollapsible(1, false);
        QList<int> list;
        list.append(9000);
        list.append(1000);
        splitter->setSizes(list);
#endif
    }

    {
        QDockWidget* dockWidget = new QDockWidget(this);
        dockWidget->setWindowTitle("Tool Widget");
        dockWidget->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
        this->addDockWidget(Qt::LeftDockWidgetArea, dockWidget);

        QFile stylesheet("data/stylesheet/toolwidget.ssa");
        if (stylesheet.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            dockWidget->setStyleSheet(QTextStream(&stylesheet).readAll());
        }
        mTool = new ToolWidget(dockWidget, *mResourceSet, QSize(192, 136));
        dockWidget->setWidget(mTool);
    }

    {
        mDriverHolder.reset(new DriverHolder());
    }

    // connection
    /// @note Maybe a sequence of connections is meaningful.
    {
        DriverHolder& driver = *mDriverHolder;
        MainDisplayWidget& disp = *mMainDisplay;
        PropertyWidget& prop = *mProperty;
        ToolWidget& tool = *mTool;
        ObjectTreeWidget& objTree = mTarget->objectTreeWidget();
        TimeLineWidget& timeLine = mTarget->timeLineWidget();
        MainMenuBar& menu = *mMainMenuBar;
        ViaPoint& via = mViaPoint;

        driver.onVisualUpdated.connect(&disp, &MainDisplayWidget::onVisualUpdated);
        tool.onVisualUpdated.connect(&disp, &MainDisplayWidget::onVisualUpdated);
        prop.onVisualUpdated.connect(&disp, &MainDisplayWidget::onVisualUpdated);
        objTree.onVisibilityUpdated.connect(&disp, &MainDisplayWidget::onVisualUpdated);
        menu.onVisualUpdated.connect(&disp, &MainDisplayWidget::onVisualUpdated);
        via.onVisualUpdated.connect(&disp, &MainDisplayWidget::onVisualUpdated);

        tool.onToolChanged.connect(&disp, &MainDisplayWidget::onToolChanged);
        tool.onViewSettingChanged.connect(&disp, &MainDisplayWidget::onViewSettingChanged);

        objTree.onSelectionChanged.connect(&driver, &DriverHolder::onSelectionChanged);
        objTree.onSelectionChanged.connect(mProperty, &PropertyWidget::onSelectionChanged);
        objTree.onSelectionChanged.connect(&timeLine, &TimeLineWidget::onSelectionChanged);

        objTree.onTreeViewUpdated.connect(&timeLine, &TimeLineWidget::onTreeViewUpdated);
        objTree.onScrollUpdated.connect(&timeLine, &TimeLineWidget::onScrollUpdated);

        timeLine.onFrameUpdated.connect(&driver, &DriverHolder::onFrameUpdated);
        timeLine.onFrameUpdated.connect(&prop, &PropertyWidget::onFrameUpdated);
        timeLine.onPlayBackStateChanged.connect(&prop, &PropertyWidget::onPlayBackStateChanged);

        menu.onProjectAttributeUpdated.connect(&disp, &MainDisplayWidget::onProjectAttributeUpdated);
        menu.onProjectAttributeUpdated.connect(&timeLine, &TimeLineWidget::onProjectAttributeUpdated);
        menu.onProjectAttributeUpdated.connect(&driver, &DriverHolder::onProjectAttributeUpdated);

        mSystem->setAnimator(*mTarget);
    }

    this->setFocusPolicy(Qt::StrongFocus);

#if 1
    auto scUndo = new QShortcut(QKeySequence("Ctrl+Z"), this);
    auto scRedo = new QShortcut(QKeySequence("Ctrl+Shift+Z"), this);
    //scUndo->setContext(Qt::WidgetWithChildrenShortcut);
    //scRedo->setContext(Qt::WidgetWithChildrenShortcut);
    scUndo->setContext(Qt::ApplicationShortcut);
    scRedo->setContext(Qt::ApplicationShortcut);

    this->connect(scUndo, &QShortcut::activated, [=](){ this->onUndoTriggered(); });
    this->connect(scRedo, &QShortcut::activated, [=](){ this->onRedoTriggered(); });
#endif
}

MainWindow::~MainWindow()
{
    closeAllProjects();
}

void MainWindow::testNewProject(const QString& aFilePath)
{
    if (mSystem)
    {
        resetProjectRefs(nullptr);

        menu::ProgressReporter progress(false, this);

        core::Project::Attribute attribute;
        auto project = mSystem->newProject(
                    aFilePath, attribute, new ProjectHook(), progress);

        if (project)
        {
            resetProjectRefs(project);
            mProjectTabBar->pushProject(*project);
        }
    }
}

void MainWindow::closeAllProjects()
{
    if (mSystem)
    {
        mProjectTabBar->removeAllProject();
        resetProjectRefs(nullptr);
        mSystem->closeAllProjects();
    }
}

void MainWindow::resetProjectRefs(core::Project* aProject)
{
    mCurrent = aProject;

    /// @note Maybe a sequence of connections is meaningful.

    if (aProject)
    {
        mDriverHolder->create(*aProject);
    }
    else
    {
        mDriverHolder->destroy();
    }

    mMainMenuBar->setProject(aProject);
    mMainDisplay->setProject(aProject);
    mTarget->setProject(aProject);
    mProperty->setProject(aProject);
    mViaPoint.setProject(aProject);

    mMainDisplay->setDriver(mDriverHolder->driver());
    mTool->setDriver(mDriverHolder->driver());
}

void MainWindow::onProjectTabChanged(core::Project& aProject)
{
    resetProjectRefs(&aProject);
}

#if 0
void MainWindow::keyPressEvent(QKeyEvent* aEvent)
{
    //qDebug() << "mainwindow: input key =" << aEvent->key() << "text =" << aEvent->text();
    bool shouldUpdate = false;

    if (mSystem && mSystem->project())
    {        
        if(aEvent->key() == Qt::Key_Z)
        {
            if (aEvent->modifiers().testFlag(Qt::ControlModifier))
            {
                if (aEvent->modifiers().testFlag(Qt::ShiftModifier))
                {
                    mSystem->project()->commandStack().redo();
                    shouldUpdate = true;
                    qDebug() << "redo";
                }
                else
                {
                    mSystem->project()->commandStack().undo();
                    shouldUpdate = true;
                    qDebug() << "undo";
                }
            }
        }
    }

    if (shouldUpdate)
    {
        mMainDisplay->updateRender();
    }
    /*
    else
    {
        mMainDisplay->throwKeyPress(aEvent);
    }
    */
}
#endif

void MainWindow::onUndoTriggered()
{
    if (mSystem && mCurrent)
    {
        auto ret = mCurrent->commandStack().undo();
        qDebug() << "undone:" << ret;
        mViaPoint.pushLog("undone : " + ret);
        mMainDisplay->updateRender();
    }
}

void MainWindow::onRedoTriggered()
{
    if (mSystem && mCurrent)
    {
        auto ret = mCurrent->commandStack().redo();
        qDebug() << "redone:" << ret;
        mViaPoint.pushLog("redone : " + ret);
        mMainDisplay->updateRender();
    }
}

void MainWindow::onNewProjectTriggered()
{
    if (mSystem)
    {
        // stop animation and main display rendering
        EventSuspender suspender(*mMainDisplay, *mTarget);

        QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "ImageFile (*.psd)");
        if (fileName.isEmpty()) return;

        // input attribute
        core::Project::Attribute attribute;
        {
            QScopedPointer<NewProjectDialog> dialog(
                        new NewProjectDialog(fileName, this));

            dialog->exec();
            if (dialog->result() != QDialog::Accepted)
            {
                return;
            }
            attribute = dialog->attribute();
        }

        // clear old project
        resetProjectRefs(nullptr);

        menu::ProgressReporter progress(false, this);

        // create
        auto project = mSystem->newProject(
                    fileName, attribute, new ProjectHook(), progress);

        if (project)
        {
            resetProjectRefs(project);
            mProjectTabBar->pushProject(*project);
        }
        else
        {
            if (mProjectTabBar->currentProject())
            {
                resetProjectRefs(mProjectTabBar->currentProject());
            }
        }
    }
}

void MainWindow::onOpenProjectTriggered()
{
    if (mSystem)
    {
        // stop animation and main display rendering
        EventSuspender suspender(*mMainDisplay, *mTarget);

        QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "ProjectFile (*.anie)");
        if (fileName.isEmpty()) return;

        // clear old project
        resetProjectRefs(nullptr);

        menu::ProgressReporter progress(false, this);

        // open
        auto project = mSystem->openProject(fileName, new ProjectHook(), progress);
        if (project)
        {
            resetProjectRefs(project);
            mProjectTabBar->pushProject(*project);
        }
        else
        {
            if (mProjectTabBar->currentProject())
            {
                resetProjectRefs(mProjectTabBar->currentProject());
            }
        }
    }
}

void MainWindow::onSaveProjectTriggered()
{
    if (mSystem && mCurrent)
    {
        // stop animation and main display rendering
        EventSuspender suspender(*mMainDisplay, *mTarget);

        if (mCurrent->isNameless())
        {
            QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "ProjectFile (*.anie)");
            if (fileName.isEmpty()) return;

            mCurrent->setFileName(fileName);
        }
        mSystem->saveProject(*mCurrent);

        mProjectTabBar->updateTabNames();
    }
}

void MainWindow::onCloseProjectTriggered()
{
    if (mSystem && mCurrent)
    {        
        mProjectTabBar->removeProject(*mCurrent);
        resetProjectRefs(nullptr);
        mSystem->closeProject(*mCurrent);

        if (mProjectTabBar->currentProject())
        {
            resetProjectRefs(mProjectTabBar->currentProject());
        }
    }
}

void MainWindow::onExportPngSeqTriggered()
{
    if (!mSystem || !mCurrent)
    {
        return;
    }
    // stop animation and main display rendering
    EventSuspender suspender(*mMainDisplay, *mTarget);

    // exporting directory
    QString dirName = QFileDialog::getExistingDirectory(this, "Exporting Folder");

    // make sure existing
    if (dirName.isEmpty()) return;
    if (!QFileInfo(dirName).exists()) return;

    // export param
    ctrl::Exporter::CommonParam cparam;
    ctrl::Exporter::PngParam pparam;
    {
        QScopedPointer<ExportDialog> dialog(
                    new ExportDialog(*mCurrent, dirName, false, this));

        dialog->exec();
        if (dialog->result() != QDialog::Accepted) return;

        cparam = dialog->commonParam();
        pparam = dialog->pngParam();
    }

    // gui for confirm overwrite
    auto overwriteConfirmer = [=](const QString&) -> bool
    {
        QMessageBox msgBox;
        msgBox.setText("File already exists.");
        msgBox.setInformativeText("Do you want to overwrite the existing file?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
        return (ret == QMessageBox::Ok);
    };

    menu::ProgressReporter progress(true, this);
    ctrl::Exporter exporter(*mCurrent);
    exporter.setOverwriteConfirmer(overwriteConfirmer);
    exporter.setProgressReporter(progress);

    // execute
    if (!exporter.execute(cparam, pparam))
    {
        return;
    }
}
void MainWindow::onExportVideoTriggered()
{
    if (!mSystem || !mCurrent)
    {
        return;
    }
    // stop animation and main display rendering
    EventSuspender suspender(*mMainDisplay, *mTarget);

    // get export file name
    const QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Exporting File"),
                QString(), // dir
                tr("Videos (*.ogv)"));

    // make sure existing
    if (fileName.isEmpty()) return;
    if (!QFileInfo(fileName).dir().exists()) return;

    // export param
    ctrl::Exporter::CommonParam cparam;
    ctrl::Exporter::VideoParam vparam;
    {
        QScopedPointer<ExportDialog> dialog(
                    new ExportDialog(*mCurrent, fileName, true, this));

        dialog->exec();
        if (dialog->result() != QDialog::Accepted) return;

        cparam = dialog->commonParam();
        vparam = dialog->videoParam();
    }

    // gui for confirm overwrite
    auto overwriteConfirmer = [=](const QString&) -> bool
    {
        QMessageBox msgBox;
        msgBox.setText("File already exists.");
        msgBox.setInformativeText("Do you want to overwrite the existing file?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
        return (ret == QMessageBox::Ok);
    };

    menu::ProgressReporter progress(true, this);
    ctrl::Exporter exporter(*mCurrent);
    exporter.setOverwriteConfirmer(overwriteConfirmer);
    exporter.setProgressReporter(progress);

    // execute
    if (!exporter.execute(cparam, vparam))
    {
        return;
    }
}

} // namespace gui
