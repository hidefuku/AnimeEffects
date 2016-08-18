#include "XC.h"
#include "cmnd/Stack.h"

namespace cmnd
{

Stack::Stack()
    : mLimit(32)
    , mCommands()
    , mCurrent(mCommands.end())
    , mMacro(NULL)
    , mSuspendCount(0)
    , mModifiable(NULL)
{
}

Stack::~Stack()
{
    XC_PTR_ASSERT(!mMacro);
    qDeleteAll(mCommands.begin(), mCommands.end());
    mCommands.clear();
}

void Stack::beginMacro(const QString& aText)
{
    (void)aText;
    XC_ASSERT(!mMacro);
    mMacro = new Macro(aText);
}

void Stack::setMacroValidLink(util::LifeLink& aLink)
{
    XC_ASSERT(mMacro);
    mMacro->setValidLink(aLink);
}

void Stack::grabMacroListener(Listener* aListener)
{
    XC_ASSERT(mMacro);
    mMacro->grabListener(aListener);
}

void Stack::endMacro()
{
    XC_PTR_ASSERT(mMacro);
    pushImpl(mMacro);
    mMacro = NULL;
}

void Stack::push(Base* aCommand)
{
    XC_PTR_ASSERT(aCommand);
    if (!aCommand) return;

    if (mMacro)
    {
        mMacro->push(aCommand);
        mModifiable = aCommand;
    }
    else
    {
        pushImpl(aCommand);
        mModifiable = aCommand;
    }
}

void Stack::push(const std::vector<Base*>&& aCommands)
{
    for (Base* command : aCommands)
    {
        push(command);
    }
}

void Stack::pushImpl(Base* aCommand)
{
    XC_PTR_ASSERT(aCommand);
    if (!aCommand) return;

    // delete invalid branch
    while (mCurrent != mCommands.end())
    {
        delete *mCurrent;
        mCurrent = mCommands.erase(mCurrent);
    }

    // make sure limit
    while (mCommands.count() >= mLimit)
    {
        delete mCommands.front();
        mCommands.pop_front();
    }

    mCommands.push_back(aCommand);

    // invoke
    if (!aCommand->isUseless())
    {
        aCommand->tryExec();
    }

    // update current
    mCurrent = mCommands.end();
}

QString Stack::undo()
{
    if (!isSuspended())
    {
        mModifiable = NULL;
        while (mCurrent != mCommands.begin())
        {
            --mCurrent;
            if (!(*mCurrent)->isUseless())
            {
                if ((*mCurrent)->tryUndo())
                {
                    return (*mCurrent)->name();
                }
            }
        }
    }
    return QString();
}

QString Stack::redo()
{
    if (!isSuspended())
    {
        mModifiable = NULL;
        while (mCurrent != mCommands.end())
        {
            if (!(*mCurrent)->isUseless())
            {
                const bool isValid = (*mCurrent)->tryRedo();
                const QString name = (*mCurrent)->name();
                ++mCurrent;
                if (isValid)
                {
                    return name;
                }
            }
            else
            {
                ++mCurrent;
            }
        }
    }
    return QString();
}

void Stack::clear()
{
    qDeleteAll(mCommands.begin(), mCommands.end());
    mCommands.clear();
    mCurrent = mCommands.end();
    mModifiable = NULL;
}

bool Stack::isModifiable(const Base* aBase) const
{
    if (!mModifiable) return false;
    return mModifiable == aBase;
}

//-------------------------------------------------------------------------------------------------
Stack::Macro::Macro(const QString& aName)
    : mCommands()
    , mHasValidLink(false)
    , mValidLink()
    , mListeners()
    , mName(aName)
{
}

Stack::Macro::~Macro()
{
    qDeleteAll(mCommands.begin(), mCommands.end());
    mCommands.clear();

    killListeners();
}

void Stack::Macro::push(Base* aCommand)
{
    XC_PTR_ASSERT(aCommand);
    if (!aCommand) return;

    mCommands.push_back(aCommand);
}

void Stack::Macro::setValidLink(util::LifeLink& aLink)
{
    mHasValidLink = true;
    mValidLink = aLink;
}

void Stack::Macro::killListeners()
{
    qDeleteAll(mListeners);
    mListeners.clear();
}

void Stack::Macro::grabListener(Listener* aListener)
{
    XC_PTR_ASSERT(aListener);
    if (aListener) // fail safe code
    {
        mListeners.push_back(aListener);
    }
}

QString Stack::Macro::name() const
{
    return mName;
}

bool Stack::Macro::tryExec()
{
    bool succeed = false;
    QMutableListIterator<Base*> itr(mCommands);
    while (itr.hasNext())
    {
        Base* command = itr.next();
        if (!command->isUseless())
        {
            if (command->tryExec())
            {
                succeed = true;
            }
        }
    }

    // call command listener
    for (auto listener : mListeners)
    {
        listener->onExecuted();
    }

    return succeed;
}

bool Stack::Macro::tryRedo()
{
    bool succeed = false;
    QMutableListIterator<Base*> itr(mCommands);
    while (itr.hasNext())
    {
        Base* command = itr.next();
        if (!command->isUseless())
        {
            if (command->tryRedo())
            {
                succeed = true;
            }
        }
    }

    // call command listener
    for (auto listener : mListeners)
    {
        listener->onRedone();
    }
    return succeed;
}

bool Stack::Macro::tryUndo()
{
    bool succeed = false;
    QMutableListIterator<Base*> itr(mCommands);
    itr.toBack();
    while (itr.hasPrevious())
    {
        Base* command = itr.previous();
        if (!command->isUseless())
        {
            if (command->tryUndo())
            {
                succeed = true;
            }
        }
    }

    // call command listener
    for (auto listener : mListeners)
    {
        listener->onUndone();
    }
    return succeed;
}

bool Stack::Macro::isUseless() const
{
    if (mHasValidLink && !mValidLink.isLinking())
    {
        return true;
    }
    if (mCommands.empty())
    {
        return true;
    }
    for (Base* command : mCommands)
    {
        if (!command->isUseless()) return false;
    }
    return true;
}

} // namespace cmnd
