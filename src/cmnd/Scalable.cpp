#include <QDebug>
#include "cmnd/Scalable.h"

namespace cmnd
{

Scalable::Scalable()
    : mCommands()
    , mListeners()
    , mExecuted(false)
{
}

Scalable::~Scalable()
{
    qDeleteAll(mCommands);
    qDeleteAll(mListeners);
}

void Scalable::grabListener(Listener* aListener)
{
    mListeners.push_back(aListener);
}

bool Scalable::isUseless() const
{
    if (!mExecuted) return false;

    for (auto command : mCommands)
    {
        if (!command->isUseless()) return false;
    }
    return true;
}

bool Scalable::initializeAndExecute()
{
    initialize();

    bool succeed = false;
    for (auto command : mCommands)
    {
        if (!command->isUseless())
        {
            if (command->tryExec()) succeed = true;
        }
    }
    return succeed;
}

bool Scalable::tryExec()
{
    const bool succeed = initializeAndExecute();
    mExecuted = true;

    // call listener
    for (auto listener : mListeners)
    {
        listener->onExecuted();
    }

    return succeed;
}

bool Scalable::tryRedo()
{
    bool succeed = false;

    for (auto command : mCommands)
    {
        if (!command->isUseless())
        {
            if (command->tryRedo()) succeed = true;
        }
    }

    // call listener
    for (auto listener : mListeners)
    {
        listener->onRedone();
    }

    return succeed;
}

bool Scalable::tryUndo()
{
    bool succeed = false;

    for (auto itr = mCommands.rbegin(); itr != mCommands.rend(); ++itr)
    {
        Base* command = *itr;
        if (!command->isUseless())
        {
            if (command->tryUndo()) succeed = true;
        }
    }

    // call listener
    for (auto listener : mListeners)
    {
        listener->onUndone();
    }

    return succeed;
}


} // namespace cmnd
