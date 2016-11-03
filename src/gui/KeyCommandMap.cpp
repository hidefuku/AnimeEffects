#include "gui/KeyCommandMap.h"

namespace gui
{

//-------------------------------------------------------------------------------------------------
KeyCommandMap::KeyCommand::KeyCommand()
    : group()
    , label()
    , binding()
    , invoker()
    , releaser()
{
}

KeyCommandMap::KeyCommand::KeyCommand(
        const QString& aGroup,
        const QString& aLabel,
        const ctrl::KeyBinding& aBinding)
    : group(aGroup)
    , label(aLabel)
    , binding(aBinding)
    , invoker()
    , releaser()
{
}

//-------------------------------------------------------------------------------------------------
KeyCommandMap::KeyCommandMap(QWidget& aParent)
    : mCommands()
    , mParent(aParent)
{
    mCommands["Undo"] =
            new KeyCommand(
                "General", "Undo a command",
                ctrl::KeyBinding(Qt::Key_Z, Qt::ControlModifier));
    mCommands["Redo"] =
            new KeyCommand(
                "General", "Redo a command",
                ctrl::KeyBinding(Qt::Key_Z, Qt::ControlModifier | Qt::ShiftModifier));
    mCommands["RotateCanvas"] =
            new KeyCommand(
                "View", "Rotate canvas",
                ctrl::KeyBinding(Qt::Key_Space, Qt::ControlModifier));
    mCommands["SelectSRT"] =
            new KeyCommand(
                "Tools", "Select srt editor",
                ctrl::KeyBinding());
}

KeyCommandMap::~KeyCommandMap()
{
    qDeleteAll(mCommands);
}

void KeyCommandMap::readFrom(const QSettings& aSrc)
{
    for (auto itr = mCommands.begin(); itr != mCommands.end(); ++itr)
    {
        readValue(aSrc, itr.key());
    }
}

void KeyCommandMap::writeTo(QSettings& aDest)
{
    for (auto itr = mCommands.begin(); itr != mCommands.end(); ++itr)
    {
        writeValue(aDest, itr.key());
    }
}

void KeyCommandMap::readValue(const QSettings& aSrc, const QString& aId)
{
    auto v = aSrc.value(aId);
    mCommands[aId]->binding.setSerialValue(v.isValid() ? v.toString() : QString());
}

void KeyCommandMap::writeValue(QSettings& aDest, const QString& aId)
{
    aDest.setValue(aId, mCommands[aId]->binding.serialValue());
}

} // namespace gui
