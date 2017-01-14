#ifndef GUI_LOCALEPARAM_H
#define GUI_LOCALEPARAM_H

#include <QString>

namespace gui
{

class LocaleParam
{
public:
    LocaleParam()
        : fontFamily()
        , fontSize()
    {}

    QString fontFamily;
    QString fontSize;
};

} // namespace gui

#endif // GUI_LOCALEPARAM_H
