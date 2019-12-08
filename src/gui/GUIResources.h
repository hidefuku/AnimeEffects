#ifndef GUI_GUIRESOURCES_H
#define GUI_GUIRESOURCES_H

#include <QString>
#include <QIcon>
#include <QPixmap>
#include <QHash>
#include "util/NonCopyable.h"

namespace gui
{

class GUIResources : private util::NonCopyable
{
public:
    GUIResources(const QString& aResourceDir);
    ~GUIResources();

    QIcon icon(const QString& aName) const;

    QString themeId() const;

private:
    typedef QHash<QString, QIcon*> IconMap;

    QString iconPath(const QString& aName) const;
    void loadIcon(const QString& aPath);

    QString mResourceDir;
    IconMap mIconMap;

    QString mThemeId;
};

} // namespace gui

#endif // GUI_GUIRESOURCES_H
