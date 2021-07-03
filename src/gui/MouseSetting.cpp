#include <QSettings>
#include <QApplication>
#include "gui/MouseSetting.h"

namespace gui
{

MouseSetting::MouseSetting()
    : invertMainViewScaling()
    , invertTimeLineScaling()
    , middleMouseMoveCanvas()
{
}

bool MouseSetting::operator==(const MouseSetting& aRhs) const
{
    return invertMainViewScaling == aRhs.invertMainViewScaling &&
            invertTimeLineScaling == aRhs.invertTimeLineScaling &&
            middleMouseMoveCanvas == aRhs.middleMouseMoveCanvas;
}

void MouseSetting::load()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QApplication::organizationName(),
                       QApplication::applicationName());
    settings.beginGroup("mousesettings");

    auto invMVScale = settings.value("InvertMainViewScaling");
    auto invTLScale = settings.value("InvertTimeLineScaling");
    auto mmMoveCanvas = settings.value("MiddleMouseMovesCanvas");

    invertMainViewScaling = invMVScale.isValid() ? invMVScale.toBool() : false;
    invertTimeLineScaling = invTLScale.isValid() ? invTLScale.toBool() : false;
    middleMouseMoveCanvas = mmMoveCanvas.isValid() ? mmMoveCanvas.toBool() : false;
}

void MouseSetting::save()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QApplication::organizationName(),
                       QApplication::applicationName());
    settings.beginGroup("mousesettings");

    settings.setValue("InvertMainViewScaling", invertMainViewScaling);
    settings.setValue("InvertTimeLineScaling", invertTimeLineScaling);
    settings.setValue("MiddleMouseMovesCanvas", middleMouseMoveCanvas);
}

} // namespace gui
