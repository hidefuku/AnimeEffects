#ifndef GUI_MAINWINDOW_H
#define GUI_MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QFileInfo>
#include <QScopedPointer>
#include "ctrl/System.h"
#include "gui/MainMenuBar.h"
#include "gui/MainDisplayWidget.h"
#include "gui/MainDisplayStyle.h"
#include "gui/TargetWidget.h"
#include "gui/PropertyWidget.h"
#include "gui/ToolWidget.h"
#include "gui/DriverHolder.h"
#include "gui/GUIResources.h"
#include "gui/ViaPoint.h"
#include "gui/ProjectTabBar.h"

namespace gui
{

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ctrl::System& aSystem, GUIResources& aResources);
    ~MainWindow();

    void showWithSettings();
    void saveCurrentSettings(int aResultCode);

    void testNewProject(const QString& aFilePath);
    void closeAllProjects();

public:
    void onNewProjectTriggered();
    void onOpenProjectTriggered();
    void onSaveProjectTriggered();
    void onCloseProjectTriggered();
    void onExportPngSeqTriggered();
    void onExportVideoTriggered();
    void onUndoTriggered();
    void onRedoTriggered();

private:
    void resetProjectRefs(core::Project* aProject);
    //virtual void keyPressEvent(QKeyEvent* aEvent);
    void onProjectTabChanged(core::Project&);

    ctrl::System& mSystem;
    GUIResources& mResources;
    ViaPoint mViaPoint;
    MainMenuBar* mMainMenuBar;
    QScopedPointer<MainDisplayStyle> mMainDisplayStyle;
    MainDisplayWidget* mMainDisplay;
    ProjectTabBar* mProjectTabBar;
    TargetWidget* mTarget;
    PropertyWidget* mProperty;
    ToolWidget* mTool;
    QScopedPointer<DriverHolder> mDriverHolder;
    core::Project* mCurrent;
};

} // namespace gui

#endif // GUI_MAINWINDOW_H
