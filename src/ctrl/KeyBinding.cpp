#include <QKeySequence>
#include "ctrl/KeyBinding.h"

namespace ctrl
{

KeyBinding::KeyBinding()
    : mKeyCode(0)
    , mModifiers(Qt::NoModifier)
{
}

KeyBinding::KeyBinding(int aKeyCode, Qt::KeyboardModifiers aModifiers)
    : mKeyCode(aKeyCode)
    , mModifiers()
{
    mModifiers = aModifiers & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier);
}

bool KeyBinding::isValidBinding() const
{
    if (0x20 <= mKeyCode && mKeyCode <= 0x7E)
    {
        return true;
    }

    switch (mKeyCode)
    {
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_Right:
    case Qt::Key_Down:
        return true;
    default:
        return false;
    }
}

bool KeyBinding::hasControlModifier() const
{
    return mModifiers & Qt::ControlModifier;
}

bool KeyBinding::hasShiftModifier() const
{
    return mModifiers & Qt::ShiftModifier;
}

bool KeyBinding::hasAltModifier() const
{
    return mModifiers & Qt::AltModifier;
}

QString KeyBinding::text() const
{
    QString t;
    if (hasControlModifier()) t += "Ctrl + ";
    if (hasShiftModifier()) t += "Shift + ";
    if (hasAltModifier()) t += "Alt + ";

    if (mKeyCode != 0)
    {
        t += QKeySequence(mKeyCode).toString();
    }
    return t;
}

bool KeyBinding::conflictsWith(const KeyBinding& aRhs) const
{
    return mKeyCode == aRhs.mKeyCode && mModifiers == aRhs.mModifiers;
}

} // namespace ctrl
