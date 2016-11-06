#ifndef GUI_KEYCOMMANDMAP_H
#define GUI_KEYCOMMANDMAP_H

#include <functional>
#include <QMap>
#include <QList>
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
        KeyCommand(const QString& aKey,
                   const QString& aGroup,
                   const QString& aLabel,
                   const ctrl::KeyBinding& aBinding);
        QString key;
        QString group;
        QString label;
        ctrl::KeyBinding binding;
        std::function<void()> invoker;
        std::function<void()> releaser;
    };

    typedef QMap<QString, KeyCommand*> MapType;
    typedef QList<KeyCommand*> ListType;

    KeyCommandMap(QWidget& aParent);
    ~KeyCommandMap();

    void readFrom(const QSettings& aSrc);
    void writeTo(QSettings& aDest);

    KeyCommand* get(const QString& aIdentifier) { return mSearchMap[aIdentifier]; }
    const KeyCommand* get(const QString& aIdentifier) const { return mSearchMap[aIdentifier]; }

    const ListType& commands() const { return mCommands; }
    const ListType& subKeyCommands() const { return mSubKeyCommands; }

private:
    void addNewKey(const QString& aKey, const QString& aGroup,
                   const QString& aName, const ctrl::KeyBinding& aBinding);
    void readValue(const QSettings& aSrc, KeyCommand& aCommand);
    void writeValue(QSettings& aDest, const KeyCommand& aCommand);
    void resetSubKeyCommands();

    ListType mCommands;
    ListType mSubKeyCommands;
    MapType mSearchMap;
    QWidget& mParent;
};

} // namespace gui

#endif // GUI_KEYCOMMANDMAP_H
