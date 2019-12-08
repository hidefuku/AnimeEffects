#ifndef GUI_PROJECTTABBAR_H
#define GUI_PROJECTTABBAR_H

#include <QVector>
#include <QTabBar>
#include "core/Project.h"

#include "gui/GUIResources.h"

namespace gui
{

class ProjectTabBar : public QTabBar
{
public:
    ProjectTabBar(QWidget* aParent, GUIResources &aResources);
    void updateTabPosition(const QSize& aDisplaySize);
    bool pushProject(core::Project& aProject);
    void removeProject(core::Project& aProject);
    void removeAllProject();
    void updateTabNames();
    core::Project* currentProject() const;
    util::Signaler<void(core::Project&)> onCurrentChanged;

    QString getTabName(const core::Project&) const;

private:
    QString getTabNameWithStatus(const core::Project&) const;
    void onTabChanged(int aIndex);

    QVector<core::Project*> mProjects;
    bool mSignal;

    GUIResources& mGUIResources;
    void onThemeUpdated();
};


} // namespace gui

#endif // GUI_PROJECTTABBAR_H
