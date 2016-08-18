#ifndef GUI_GUIRESOURCESET_H
#define GUI_GUIRESOURCESET_H

#include <QString>
#include <QIcon>
#include <QPixmap>
#include <QHash>
#include "util/NonCopyable.h"

namespace gui
{

class GUIResourceSet : private util::NonCopyable
{
public:
    GUIResourceSet(const QString& aResourceDir);
    ~GUIResourceSet();

    QIcon icon(const QString& aName) const;

private:
    typedef QHash<QString, QIcon*> IconMap;

    QString iconPath(const QString& aName) const;
    void loadIcon(const QString& aPath);

    QString mResourceDir;
    IconMap mIconMap;
};

} // namespace gui

#endif // GUI_GUIRESOURCESET_H
