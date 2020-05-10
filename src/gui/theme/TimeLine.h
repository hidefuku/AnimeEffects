#ifndef GUI_THEME_TIMELINE_H
#define GUI_THEME_TIMELINE_H

#include <QColor>
#include <QPalette>

namespace theme
{

/*
 * Used to pass theme values to the ctrl::time::time_Renderer
 */
class TimeLine
{

public:
    TimeLine();

    void reset();

    QColor headerContentColor() const;
    void setHeaderContentColor(const QColor &headerContentColor);

    QColor headerBackgroundColor() const;
    void setHeaderBackgroundColor(const QColor &headerBackgroundColor);

    QColor trackColor() const;
    void setTrackColor(const QColor &trackColor);

    QColor trackEdgeColor() const;
    void setTrackEdgeColor(const QColor &trackEdgeColor);

    QColor trackTextColor() const;
    void setTrackTextColor(const QColor &trackTextColor);

    QColor trackSelectColor() const;
    void setTrackSelectColor(const QColor &trackSelectColor);

    QColor trackSeperatorColor() const;
    void setTrackSeperatorColor(const QColor &trackSeperatorColor);

private:
    void shade();

    QColor mHeaderContentColor;
    QColor mHeaderBackgroundColor;

    QColor mTrackColor;
    QColor mTrackEdgeColor;
    QColor mTrackTextColor;
    QColor mTrackSelectColor;
    QColor mTrackSeperatorColor;
};

} // namespace theme

#endif // GUI_THEME_TIMELINE_H
