#ifndef GUI_WIDGETMETRICS
#define GUI_WIDGETMETRICS

#include <QWidget>
#include "core/GraphicsMetrics.h"
#include "util/NonCopyable.h"

namespace gui
{

class WidgetMetrics
        : public core::GraphicsMetrics
        , private util::NonCopyable
{
public:
    WidgetMetrics(QWidget& aWidget)
        : mWidget(aWidget)
    {
    }

    virtual QRect boundingRect(const QString& aText) const
    {
        return mWidget.fontMetrics().boundingRect(aText);
    }

private:
    QWidget& mWidget;
};

}

#endif // GUI_WIDGETMETRICS

