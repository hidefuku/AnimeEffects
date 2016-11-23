#ifndef GUI_MAINMENUBAR_H
#define GUI_MAINMENUBAR_H

#include <QMenuBar>
#include <QAction>
#include "core/Project.h"
namespace gui { class MainWindow; }
namespace gui { class ViaPoint; }
namespace gui
{

class MainMenuBar : public QMenuBar
{
    Q_OBJECT
public:
    MainMenuBar(MainWindow& aMainWindow, ViaPoint& aViaPoint, QWidget* aParent);
    void setProject(core::Project* aProject);
    void setShowResourceWindow(bool aShow);

public:
    // signals
    util::Signaler<void()> onVisualUpdated;
    util::Signaler<void()> onProjectAttributeUpdated;

private:
    void onCanvasSizeTriggered();
    void onMaxFrameTriggered();
    void onLoopTriggered();
    bool confirmMaxFrameUpdating(int aNewMaxFrame) const;

    ViaPoint& mViaPoint;
    core::Project* mProject;
    QVector<QAction*> mProjectActions;
    QAction* mShowResourceWindow;
};

} // namespace gui

#endif // GUI_MAINMENUBAR_H
