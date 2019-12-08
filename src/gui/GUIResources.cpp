#include "gui/GUIResources.h"

namespace gui
{

GUIResources::GUIResources(const QString& aResourceDir)
    : mResourceDir(aResourceDir)
    , mIconMap()
    , mThemeMap()
    , mThemeId("default")
{
    detectThemes();

    QSettings settings;
    auto theme = settings.value("generalsettings/ui/theme");
    if (theme.isValid())
    {
        setTheme(theme.toString());
    }

    loadIcons();

}

GUIResources::~GUIResources()
{
    for (IconMap::iterator itr = mIconMap.begin(); itr != mIconMap.end(); ++itr)
    {
        QIcon* icon = *itr;
        delete icon;
    }
}

QIcon GUIResources::icon(const QString& aName) const
{
    QIcon* icon = mIconMap[aName];
    if (icon)
    {
        return *icon;
    }
    else
    {
        XC_ASSERT(0);
        return QIcon();
    }
}

QString GUIResources::iconPath(const QString& aName)
{
    return themePath()+"/icon/"+aName+".png";
}

void GUIResources::loadIcon(const QString& aPath)
{
    QString name = QFileInfo(aPath).baseName();
    QPixmap source(aPath);

    QIcon* icon = new QIcon();
    mIconMap[name] = icon;
    icon->addPixmap(source, QIcon::Normal, QIcon::Off);

#if 0
    {
        QPixmap work = source;
        QPainter painter(&work);
        //painter.setCompositionMode(QPainter::CompositionMode_Multiply);
        painter.setCompositionMode(QPainter::CompositionMode_Screen);
        painter.fillRect(work.rect(), QColor(128, 128, 128, 128));
        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        painter.drawPixmap(source.rect(), source);
        painter.end();
        //icon->addPixmap(work, QIcon::Selected, QIcon::On);
        icon->addPixmap(work, QIcon::Disabled, QIcon::Off);
    }
#endif
}

void GUIResources::loadIcons()
{
    if(!mIconMap.empty())
    {
        QHashIterator<QString, QIcon*> i(mIconMap);
        while (i.hasNext())
        {
            i.next();
            QIcon* icon = i.value();
            //qDebug() << i.key() << ": " << i.value();
            delete icon;
        }
        mIconMap.clear();
    }

    const QString iconDirPath(mResourceDir+"/themes/"+mThemeId+"/icon");

    QStringList filters;
    filters << "*.png";
    QDirIterator itr(iconDirPath, filters, QDir::Files, QDirIterator::Subdirectories);

    while (itr.hasNext())
    {
        loadIcon(itr.next());
    }
}

void GUIResources::detectThemes()
{
    const QString themesDirPath(mResourceDir+"/themes");

    QDirIterator itr(themesDirPath, QDir::Dirs, QDirIterator::FollowSymlinks);

    while (itr.hasNext())
    {
        itr.next();
        if(itr.fileName() != "." && itr.fileName() != "..")
        {
            qDebug() << itr.fileName();
            mThemeMap.insert(itr.fileName(), itr.fileInfo());
        }
    }
}

QString GUIResources::themeId() const
{
    return mThemeId;
}

QString GUIResources::themePath()
{
    return mThemeMap[mThemeId].absoluteFilePath();
}

QStringList GUIResources::themeList()
{
    QStringList theThemeList;
    if(!mThemeMap.empty())
    {
        QHashIterator<QString, QFileInfo> i(mThemeMap);
        while (i.hasNext()) {
            i.next();
            theThemeList.append(i.key());
        }
    }
    return theThemeList;
}

bool GUIResources::hasTheme(const QString &aThemeId)
{
    return mThemeMap.contains(aThemeId);
}

void GUIResources::setTheme(const QString &aThemeId)
{
    if(mThemeId != aThemeId) {
        mThemeId = aThemeId;
        loadIcons();
        onThemeChanged();
    }
}

} // namespace gui
