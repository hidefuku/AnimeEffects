#ifndef CORE_OBJECTNODEBOX_H
#define CORE_OBJECTNODEBOX_H

#include <QRectF>

namespace core
{

class ObjectNodeBox
{
public:
    ObjectNodeBox()
        : rect()
        , labelRect()
        , isPack(false)
        , isFocusing(false)
    {
    }

    QRectF rect;
    QRectF labelRect;
    bool isPack;
    bool isFocusing;
};

} // namespace core

#endif // CORE_OBJECTNODEBOX_H
