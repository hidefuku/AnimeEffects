#include "gui/KeyCommandMap.h"

namespace gui
{

//-------------------------------------------------------------------------------------------------
KeyCommandMap::KeyCommand::KeyCommand()
    : key()
    , group()
    , label()
    , binding()
    , invoker()
    , releaser()
{
}

KeyCommandMap::KeyCommand::KeyCommand(
        const QString& aKey,
        const QString& aGroup,
        const QString& aLabel,
        const ctrl::KeyBinding& aBinding)
    : key(aKey)
    , group(aGroup)
    , label(aLabel)
    , binding(aBinding)
    , invoker()
    , releaser()
{
}

//-------------------------------------------------------------------------------------------------
KeyCommandMap::KeyCommandMap(QWidget& aParent)
    : QObject(&aParent)
    , mCommands()
    , mSubKeyCommands()
    , mSearchMap()
    , mParent(aParent)
{
    auto general = tr("General");
    auto view = tr("View");
    auto tools = tr("Tools");

    addNewKey("Undo", general, tr("Undo last action"),
              ctrl::KeyBinding(Qt::Key_Z, Qt::ControlModifier));

    addNewKey("Redo", general, tr("Redo last action"),
              ctrl::KeyBinding(Qt::Key_Z, Qt::ControlModifier | Qt::ShiftModifier));

    addNewKey("SaveProject", general, tr("Save project"),
              ctrl::KeyBinding(Qt::Key_S, Qt::ControlModifier));

    addNewKey("MoveCanvas", view, tr("Move canvas"),
              ctrl::KeyBinding(Qt::Key_Space, Qt::ControlModifier));

    addNewKey("RotateCanvas", view, tr("Rotate canvas"),
              ctrl::KeyBinding(Qt::Key_Space, Qt::ControlModifier | Qt::ShiftModifier));

    addNewKey("ResetCanvasAngle", view, tr("Reset canvas angle"),
              ctrl::KeyBinding(Qt::Key_Space, Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_F1));

    addNewKey("SelectCursor", tools, tr("Select cursor tool"),
              ctrl::KeyBinding());

    addNewKey("SelectSRT", tools, tr("Select SRT editor"),
              ctrl::KeyBinding());

    addNewKey("SelectBone", tools, tr("Select bone editor"),
              ctrl::KeyBinding());

    addNewKey("SelectPose", tools, tr("Select pose editor"),
              ctrl::KeyBinding());

    addNewKey("SelectMesh", tools, tr("Select mesh editor"),
              ctrl::KeyBinding());

    addNewKey("SelectFFD", tools, tr("Select FFD editor"),
              ctrl::KeyBinding());

    resetSubKeyCommands();
}

KeyCommandMap::~KeyCommandMap()
{
    qDeleteAll(mCommands);
}

void KeyCommandMap::addNewKey(const QString& aKey, const QString& aGroup,
                              const QString& aName, const ctrl::KeyBinding& aBinding)
{
    auto command = new KeyCommand(aKey, aGroup, aName, aBinding);
    mSearchMap[aKey] = command;
    mCommands.push_back(command);
}

void KeyCommandMap::readFrom(const QSettings& aSrc)
{
    for (auto command : mCommands)
    {
        readValue(aSrc, *command);
    }
    resetSubKeyCommands();
}

void KeyCommandMap::writeTo(QSettings& aDest)
{
    for (auto command : mCommands)
    {
        writeValue(aDest, *command);
    }
    resetSubKeyCommands();
}

void KeyCommandMap::readValue(const QSettings& aSrc, KeyCommand& aCommand)
{
    auto v = aSrc.value(aCommand.key);
    aCommand.binding.setSerialValue(v.isValid() ? v.toString() : QString());
}

void KeyCommandMap::writeValue(QSettings& aDest, const KeyCommand& aCommand)
{
    aDest.setValue(aCommand.key, aCommand.binding.serialValue());
}

void KeyCommandMap::resetSubKeyCommands()
{
    mSubKeyCommands.clear();

    for (auto command : mCommands)
    {
        if (command->binding.hasSubKeyCode())
        {
            mSubKeyCommands.push_back(command);
        }
    }
}

} // namespace gui
