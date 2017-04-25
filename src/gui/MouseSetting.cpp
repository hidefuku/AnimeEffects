#include <QSettings>
#include <QApplication>
#include "gui/MouseSetting.h"

namespace gui
{

MouseSetting::MouseSetting()
    : invertMainViewScaling()
    , invertTimeLineScaling()
{
}

bool MouseSetting::operator==(const MouseSetting& aRhs) const
{
    return invertMainViewScaling == aRhs.invertMainViewScaling &&
            invertTimeLineScaling == aRhs.invertTimeLineScaling;
}

void MouseSetting::load()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QApplication::organizationName(),
                       QApplication::applicationName());
    settings.beginGroup("mousesettings");

    auto invMVScale = settings.value("InvertMainViewScaling");
    auto invTLScale = settings.value("InvertTimeLineScaling");

    invertMainViewScaling = invMVScale.isValid() ? invMVScale.toBool() : false;
    invertTimeLineScaling = invTLScale.isValid() ? invTLScale.toBool() : false;
}

void MouseSetting::save()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QApplication::organizationName(),
                       QApplication::applicationName());
    settings.beginGroup("mousesettings");

    settings.setValue("InvertMainViewScaling", invertMainViewScaling);
    settings.setValue("InvertTimeLineScaling", invertTimeLineScaling);
}

} // namespace gui
