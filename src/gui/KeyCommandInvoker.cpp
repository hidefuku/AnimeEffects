#include <QDebug>
#include "gui/KeyCommandInvoker.h"
#include "gui/KeyCommandMap.h"

namespace gui
{

KeyCommandInvoker::KeyCommandInvoker(KeyCommandMap& aMap)
    : mMap(aMap)
    , mLastCommand(nullptr)
{
}

void KeyCommandInvoker::onKeyPressed(const QKeyEvent* aEvent)
{
    if (aEvent->isAutoRepeat()) return;

    // release the previous command
    if (mLastCommand)
    {
        if (mLastCommand->releaser)
        {
            mLastCommand->releaser();
        }
        mLastCommand = nullptr;
    }

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
            // invoke
            mLastCommand = command;
            command->invoker();
            break;
        }
    }
}

void KeyCommandInvoker::onKeyReleased(const QKeyEvent* aEvent)
{
    //qDebug() << "rls" << aEvent->key() << aEvent->modifiers() << aEvent->isAutoRepeat();
    if (aEvent->isAutoRepeat()) return;

    if (mLastCommand)
    {
        if (mLastCommand->releaser)
        {
            mLastCommand->releaser();
        }
        mLastCommand = nullptr;
    }
}

} // namespace gui
