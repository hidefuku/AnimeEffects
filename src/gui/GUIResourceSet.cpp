#include "gui/GUIResourceSet.h"
#include <QPainter>
#include <QColor>
#include <QDirIterator>
#include <QStringList>
#include "XC.h"

namespace gui
{

GUIResourceSet::GUIResourceSet(const QString& aResourceDir)
    : mResourceDir(aResourceDir)
    , mIconMap()
{
    const QString iconDirPath(mResourceDir + "/icon");

    QStringList filters;
    filters << "*.png";
    QDirIterator itr(iconDirPath, filters, QDir::Files, QDirIterator::Subdirectories);

    while (itr.hasNext())
    {
        loadIcon(itr.next());
    }
}

GUIResourceSet::~GUIResourceSet()
{
    for (IconMap::iterator itr = mIconMap.begin(); itr != mIconMap.end(); ++itr)
    {
        QIcon* icon = *itr;
        delete icon;
    }
}

QIcon GUIResourceSet::icon(const QString& aName) const
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

QString GUIResourceSet::iconPath(const QString& aName) const
{
    return mResourceDir + "/icon/" + aName + ".png";
}

void GUIResourceSet::loadIcon(const QString& aPath)
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

} // namespace gui
