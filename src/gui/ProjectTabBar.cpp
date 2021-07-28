#include <QFile>
#include <QFileInfo>
#include "gui/ProjectTabBar.h"

namespace gui
{

//-------------------------------------------------------------------------------------------------
ProjectTabBar::ProjectTabBar(QWidget* aParent, GUIResources& aResources)
    : QTabBar(aParent)
    , mProjects()
    , mSignal(true)
    , mGUIResources(aResources)
{
    static const int kHeight = 18;
    this->setGeometry(0, 0, aParent->geometry().width(), kHeight);
    this->setUsesScrollButtons(false);
    this->setAutoFillBackground(true);
    this->setExpanding(false);
    this->setElideMode(Qt::ElideRight);
    this->setDrawBase(false);

    this->connect(this, &QTabBar::currentChanged, this, &ProjectTabBar::onTabChanged);
    mGUIResources.onThemeChanged.connect(this, &ProjectTabBar::onThemeUpdated);
}

void ProjectTabBar::updateTabPosition(const QSize& aDisplaySize)
{
    static const int kHeight = 18;
    this->setGeometry(0, 0, aDisplaySize.width(), kHeight);
}

QString ProjectTabBar::getTabName(const core::Project& aProject) const
{
    QString name = aProject.fileName();
    return name.isEmpty() ? QString("New Project") : QFileInfo(name).fileName();
}

void ProjectTabBar::onThemeUpdated(theme::Theme& aTheme)
{
    QFile stylesheet(aTheme.path()+"/stylesheet/modetabbar.ssa");
    if (stylesheet.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        this->setStyleSheet(QTextStream(&stylesheet).readAll());
    }
}

QString ProjectTabBar::getTabNameWithStatus(const core::Project& aProject) const
{
    QString name = getTabName(aProject);
    if (aProject.commandStack().isEdited())
    {
        name += "*";
    }
    return name;
}

bool ProjectTabBar::pushProject(core::Project& aProject)
{
    if (!mProjects.contains(&aProject))
    {
        mSignal = false;
        mProjects.push_back(&aProject);
        const int index = this->addTab(getTabNameWithStatus(aProject));
        this->setCurrentIndex(index);
        mSignal = true;

        aProject.commandStack().setOnEditStatusChanged(
                    [=](bool) { this->updateTabNames(); });
        return true;
    }
    return false;
}

void ProjectTabBar::removeProject(core::Project& aProject)
{
    const int index = mProjects.indexOf(&aProject);

    if (0 <= index && index < mProjects.count())
    {
        mSignal = false;

        mProjects.removeOne(&aProject);
        this->removeTab(index);

        const int tabCount = this->count();
        if (tabCount > 0)
        {
            if (index < tabCount)
            {
                this->setCurrentIndex(index);
            }
            else
            {
                this->setCurrentIndex(tabCount - 1);
            }
        }
        mSignal = true;

        aProject.commandStack().setOnEditStatusChanged(nullptr);
    }
}

void ProjectTabBar::removeAllProject()
{
    mSignal = false;
    for (int i = 0; i < mProjects.count(); ++i)
    {
        this->removeTab(0);
    }
    mProjects.clear();
    mSignal = true;
}

void ProjectTabBar::updateTabNames()
{
    for (int i = 0; i < mProjects.count(); ++i)
    {
        this->setTabText(i, getTabNameWithStatus(*mProjects[i]));
    }
}

core::Project* ProjectTabBar::currentProject() const
{
    const int index = this->currentIndex();
    if (0 <= index && index < mProjects.count())
    {
        return mProjects[index];
    }
    return nullptr;
}

void ProjectTabBar::onTabChanged(int aIndex)
{
    if (mSignal)
    {
        if (0 <= aIndex && aIndex < mProjects.count())
        {
            onCurrentChanged(*mProjects[aIndex]);
        }
    }
}

} // namespace gui
