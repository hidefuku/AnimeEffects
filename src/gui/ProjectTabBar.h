#ifndef GUI_PROJECTTABBAR_H
#define GUI_PROJECTTABBAR_H

#include <QVector>
#include <QTabBar>
#include "core/Project.h"

namespace gui
{

class ProjectTabBar : public QTabBar
{
public:
    ProjectTabBar(QWidget* aParent);
    void updateTabPosition(const QSize& aDisplaySize);
    bool pushProject(core::Project& aProject);
    void removeProject(core::Project& aProject);
    void removeAllProject();
    void updateTabNames();
    core::Project* currentProject() const;
    util::Signaler<void(core::Project&)> onCurrentChanged;

private:
    QString getTabName(const core::Project&) const;
    void onTabChanged(int aIndex);

    QVector<core::Project*> mProjects;
    bool mSignal;
};


} // namespace gui

#endif // GUI_PROJECTTABBAR_H
