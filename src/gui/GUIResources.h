#ifndef GUI_GUIRESOURCES_H
#define GUI_GUIRESOURCES_H

#include <QString>
#include <QIcon>
#include <QFileInfo>
#include <QPixmap>
#include <QHash>
#include <QSettings>
#include <QPainter>
#include <QColor>
#include <QDirIterator>
#include <QStringList>

#include "XC.h"
#include "util/NonCopyable.h"
#include "util/Signaler.h"

namespace gui
{

class GUIResources : private util::NonCopyable
{
public:
    GUIResources(const QString& aResourceDir);
    ~GUIResources();

    QIcon icon(const QString& aName) const;

    QString themeId() const;
    QString themePath();
    QStringList themeList();
    bool hasTheme(const QString& aThemeId);
    void setTheme(const QString& aThemeId);

public:
    // signals
    util::Signaler<void()> onThemeChanged;

private:
    typedef QHash<QString, QIcon*> IconMap;
    typedef QHash<QString, QFileInfo> ThemeMap;

    void loadIcons();
    void detectThemes();

    QString iconPath(const QString& aName);
    void loadIcon(const QString& aPath);

    QString mResourceDir;
    IconMap mIconMap;

    ThemeMap mThemeMap;
    QString mThemeId;
};

} // namespace gui

#endif // GUI_GUIRESOURCES_H
