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
#include "theme/Theme.h"

namespace gui
{

class GUIResources : private util::NonCopyable
{
public:
    GUIResources(const QString& aResourceDir);
    ~GUIResources();

    QIcon icon(const QString& aName) const;

    QStringList themeList();
    bool hasTheme(const QString& aThemeId);
    void setTheme(const QString& aThemeId);

public:
    // signals
    util::Signaler<void(theme::Theme&)> onThemeChanged;

    void triggerOnThemeChanged();

private:
    typedef QHash<QString, QIcon*> IconMap;
    typedef QHash<QString, theme::Theme> ThemeMap;

    void loadIcons();
    void detectThemes();

    QString iconPath(const QString& aName);
    void loadIcon(const QString& aPath);

    QString mResourceDir;
    IconMap mIconMap;

    ThemeMap mThemeMap;

    theme::Theme mTheme;
};

} // namespace gui

#endif // GUI_GUIRESOURCES_H
