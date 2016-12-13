#include "XC.h"
#include "cmnd/Stack.h"

namespace cmnd
{

Stack::Stack()
    : mLimit(32)
    , mCommands()
    , mCurrent(mCommands.end())
    , mMacro()
    , mSuspendCount(0)
    , mModifiable()
    , mEditingOrigin(0)
    , mIsEdited()
    , mOnEditStatusChanged()
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
    {
        if (!aCommand->isUseless())
        {
            aCommand->tryExec();
        }
        --mEditingOrigin; // update editing origin
    }

    // update current
    mCurrent = mCommands.end();

    updateEditStatus();
}

QString Stack::undo(bool* undone)
{
    if (undone) *undone = false;

    if (!isSuspended())
    {
        mModifiable = NULL;
        while (mCurrent != mCommands.begin())
        {
            --mCurrent;

            bool success = false;
            if (!(*mCurrent)->isUseless())
            {
                success = (*mCurrent)->tryUndo();
            }
            ++mEditingOrigin; // update editing origin

            if (success)
            {
                updateEditStatus();
                if (undone) *undone = true;
                return (*mCurrent)->name();
            }
        }
    }

    updateEditStatus();
    return QString();
}

QString Stack::redo(bool* redone)
{
    if (redone) *redone = false;

    if (!isSuspended())
    {
        mModifiable = NULL;
        while (mCurrent != mCommands.end())
        {
            bool success = false;
            QString name;

            if (!(*mCurrent)->isUseless())
            {
                success = (*mCurrent)->tryRedo();
                name = (*mCurrent)->name();
            }
            ++mCurrent; // update current
            --mEditingOrigin; // update editing origin

            if (success)
            {
                updateEditStatus();
                if (redone) *redone = true;
                return name;
            }
        }
    }

    updateEditStatus();
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

void Stack::resetEditingOrigin()
{
    mEditingOrigin = 0;
    updateEditStatus();
}

bool Stack::isEdited() const
{
    return mIsEdited;
}

void Stack::setOnEditStatusChanged(const std::function<void(bool)>& aFunction)
{
    mOnEditStatusChanged = aFunction;
}

void Stack::updateEditStatus()
{
    const bool isEdited = (mEditingOrigin != 0);

    if (mIsEdited != isEdited)
    {
        mIsEdited = isEdited;

        if (mOnEditStatusChanged)
        {
            mOnEditStatusChanged(isEdited);
        }
    }
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
