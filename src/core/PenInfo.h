#ifndef CORE_PENINFO_H
#define CORE_PENINFO_H

#include <QVector2D>

namespace core
{

class PenInfo
{
public:
    PenInfo()
        : screenPos()
        , screenVel()
        , pos()
        , vel()
        , pressure(0.0f)
        , isPressing(false)
        , screenWidth(0)
        , screenHeight(0)
        , id(0)
    {
    }

    QVector2D screenPos;
    QVector2D screenVel;
    QVector2D pos;
    QVector2D vel;
    float pressure;
    bool isPressing;
    int screenWidth;
    int screenHeight;
    int id;
};

} // namespace core

#endif // CORE_PENINFO_H
