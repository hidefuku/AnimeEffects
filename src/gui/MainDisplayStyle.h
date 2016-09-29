#ifndef GUI_MAINDISPLAYSTYLE
#define GUI_MAINDISPLAYSTYLE

#include <QWidget>
#include "util/NonCopyable.h"
#include "ctrl/GraphicStyle.h"
#include "gui/GUIResources.h"

namespace gui
{

class MainDisplayStyle
        : public ctrl::GraphicStyle
        , private util::NonCopyable
{
public:
    MainDisplayStyle(QWidget& aWidget, GUIResources& aResources)
        : mWidget(aWidget)
        , mResources(aResources)
    {
    }

    virtual QFont font() const
    {
        return mWidget.font();
    }

    virtual QRect boundingRect(const QString& aText) const
    {
        return mWidget.fontMetrics().boundingRect(aText);
    }

    virtual QIcon icon(const QString& aName) const
    {
        return mResources.icon(aName);
    }

private:
    QWidget& mWidget;
    GUIResources& mResources;
};

}

#endif // GUI_MAINDISPLAYSTYLE

