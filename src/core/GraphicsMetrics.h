#ifndef CORE_GRAPHICSMETRICS
#define CORE_GRAPHICSMETRICS

#include <QRect>
#include <QString>

namespace core
{

class GraphicsMetrics
{
public:
    virtual ~GraphicsMetrics() {}
    virtual QRect boundingRect(const QString& aText) const = 0;
};

}

#endif // CORE_GRAPHICSMETRICS

