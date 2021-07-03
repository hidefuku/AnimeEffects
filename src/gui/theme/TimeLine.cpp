#include "gui/theme/TimeLine.h"

namespace theme
{

//-------------------------------------------------------------------------------------------------
TimeLine::TimeLine()
    : mHeaderContentColor(QColor(60, 60, 70, 255))
    , mHeaderBackgroundColor(QColor(160, 160, 160, 255))
    , mTrackColor(QColor(250, 250, 250, 255))
    , mTrackEdgeColor(QColor(190, 190, 190, 255))
    , mTrackTextColor(QColor(170, 170, 170, 255))
    , mTrackSelectColor(QColor(235, 240, 250, 255))
    , mTrackSeperatorColor(QColor(200, 200, 205, 255))
{
    reset();
}

void TimeLine::reset()
{
    QPalette palette;
    qreal lightness = palette.window().color().lightnessF();

    if(lightness > 0.5) { // Light theme
        mHeaderContentColor = QColor(60, 60, 70, 255);
        mHeaderBackgroundColor = QColor(160, 160, 160, 255);

        mTrackColor = QColor(250, 250, 250, 255);
        mTrackEdgeColor = QColor(190, 190, 190, 255);
        mTrackTextColor = QColor(170, 170, 170, 255);
        mTrackSelectColor = QColor(235, 240, 250, 255);
        mTrackSeperatorColor = QColor(200, 200, 205, 255);
    } else { // Dark theme
        mHeaderContentColor = palette.text().color();
        mHeaderBackgroundColor = palette.window().color();

        mTrackColor = QColor(46, 47, 48, 255);
        mTrackEdgeColor = QColor(64, 65, 66, 255);
        mTrackTextColor = palette.text().color(); //QColor(44, 45, 46, 255);
        mTrackSelectColor = QColor(58, 59, 60, 255);
        mTrackSeperatorColor = QColor(74, 75, 76, 255);
    }
    shade();
}

void TimeLine::shade()
{
    mTrackColor = QColor(mTrackColor.red(), mTrackColor.green(), mTrackColor.blue(), 180);
    mTrackEdgeColor = QColor(mTrackEdgeColor.red(), mTrackEdgeColor.green(), mTrackEdgeColor.blue(), 180);
    mTrackTextColor = QColor(mTrackTextColor.red(), mTrackTextColor.green(), mTrackTextColor.blue(), 180);
    mTrackSelectColor = QColor(mTrackSelectColor.red(), mTrackSelectColor.green(), mTrackSelectColor.blue(), 180);
    mTrackSeperatorColor = QColor(mTrackSeperatorColor.red(), mTrackSeperatorColor.green(), mTrackSeperatorColor.blue(), 180);
}

QColor TimeLine::headerContentColor() const
{
    return mHeaderContentColor;
}

void TimeLine::setHeaderContentColor(const QColor &headerContentColor)
{
    mHeaderContentColor = headerContentColor;
}

QColor TimeLine::headerBackgroundColor() const
{
    return mHeaderBackgroundColor;
}

void TimeLine::setHeaderBackgroundColor(const QColor &headerBackgroundColor)
{
    mHeaderBackgroundColor = headerBackgroundColor;
}

QColor TimeLine::trackSeperatorColor() const
{
    return mTrackSeperatorColor;
}

void TimeLine::setTrackSeperatorColor(const QColor &trackSeperatorColor)
{
    mTrackSeperatorColor = trackSeperatorColor;
}

QColor TimeLine::trackTextColor() const
{
    return mTrackTextColor;
}

void TimeLine::setTrackTextColor(const QColor &trackTextColor)
{
    mTrackTextColor = trackTextColor;
}

QColor TimeLine::trackSelectColor() const
{
    return mTrackSelectColor;
}

void TimeLine::setTrackSelectColor(const QColor &trackSelectColor)
{
    mTrackSelectColor = trackSelectColor;
}

QColor TimeLine::trackEdgeColor() const
{
    return mTrackEdgeColor;
}

void TimeLine::setTrackEdgeColor(const QColor &trackEdgeColor)
{
    mTrackEdgeColor = trackEdgeColor;
}

QColor TimeLine::trackColor() const
{
    return mTrackColor;
}

void TimeLine::setTrackColor(const QColor &trackColor)
{
    mTrackColor = trackColor;
    shade();
}

} // namespace theme
