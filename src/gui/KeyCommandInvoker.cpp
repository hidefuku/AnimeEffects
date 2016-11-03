#include "gui/KeyCommandInvoker.h"
#include "gui/KeyCommandMap.h"

namespace gui
{

KeyCommandInvoker::KeyCommandInvoker(KeyCommandMap& aMap)
    : mMap(aMap)
    , mLastCommand(nullptr)
{
}

void KeyCommandInvoker::onKeyPressed(QKeyEvent* aEvent)
{
    mLastCommand = nullptr;

    if (aEvent->isAutoRepeat()) return;

    const ctrl::KeyBinding keyBind(aEvent->key(), aEvent->modifiers());
    if (!keyBind.isValidBinding()) return;

    // each key commands
    for (auto itr = mMap.map().begin(); itr != mMap.map().end(); ++itr)
    {
        auto command = itr.value();

        if (command->invoker &&
                command->binding.isValidBinding() &&
                command->binding.matchesExactlyWith(keyBind))
        {
            mLastCommand = command;
            command->invoker();
            break;
        }
    }
}

void KeyCommandInvoker::onKeyReleased(QKeyEvent* aEvent)
{
    if (mLastCommand && mLastCommand->releaser)
    {
        const ctrl::KeyBinding keyBind(aEvent->key(), aEvent->modifiers());
        if (!keyBind.isValidBinding() || !mLastCommand->binding.matchesExactlyWith(keyBind))
        {
            mLastCommand->releaser();
            mLastCommand = nullptr;
        }
    }
}

} // namespace gui
