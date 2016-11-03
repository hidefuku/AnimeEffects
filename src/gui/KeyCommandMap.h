#ifndef GUI_KEYCOMMANDMAP_H
#define GUI_KEYCOMMANDMAP_H

#include <functional>
#include <QMap>
#include <QShortcut>
#include <QSettings>
#include "ctrl/KeyBinding.h"

namespace gui
{

class KeyCommandMap
{
public:
    struct KeyCommand
    {
        KeyCommand();
        KeyCommand(const QString& aGroup,
                   const QString& aLabel,
                   const ctrl::KeyBinding& aBinding);
        QString group;
        QString label;
        ctrl::KeyBinding binding;
        std::function<void()> invoker;
        std::function<void()> releaser;
    };

    typedef QMap<QString, KeyCommand*> MapType;

    KeyCommandMap(QWidget& aParent);
    ~KeyCommandMap();

    void readFrom(const QSettings& aSrc);
    void writeTo(QSettings& aDest);

    KeyCommand* get(const QString& aIdentifier) { return mCommands[aIdentifier]; }
    const KeyCommand* get(const QString& aIdentifier) const { return mCommands[aIdentifier]; }

    MapType& map() { return mCommands; }
    const MapType& map() const { return mCommands; }

private:
    void readValue(const QSettings& aSrc, const QString& aId);
    void writeValue(QSettings& aDest, const QString& aId);

    MapType mCommands;
    QWidget& mParent;
};

} // namespace gui

#endif // GUI_KEYCOMMANDMAP_H
