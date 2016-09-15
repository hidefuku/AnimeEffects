#ifndef CTRL_GRAPHICSTYLE_H
#define CTRL_GRAPHICSTYLE_H

#include <QRect>
#include <QString>
#include <QIcon>

namespace ctrl
{

class GraphicStyle
{
public:
    virtual ~GraphicStyle() {}
    virtual QRect boundingRect(const QString& aText) const = 0;
    virtual QIcon icon(const QString& aName) const = 0;
};

}

#endif // CTRL_GRAPHICSTYLE_H
