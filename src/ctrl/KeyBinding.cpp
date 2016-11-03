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

bool KeyBinding::matchesExactlyWith(const KeyBinding& aRhs) const
{
    return mKeyCode == aRhs.mKeyCode && mModifiers == aRhs.mModifiers;
}

void KeyBinding::setSerialValue(const QString& aValue)
{
    const QStringList v = aValue.split(',');
    if (v.size() < 2) return;

    bool success = false;

    const int key = v[0].toInt(&success);
    if (!success) return;

    const int mod = v[1].toInt(&success);
    if (!success) return;

    mKeyCode = key;
    mModifiers = Qt::NoModifier;
    if (mod & 0x01) mModifiers |= Qt::ControlModifier;
    if (mod & 0x02) mModifiers |= Qt::ShiftModifier;
    if (mod & 0x04) mModifiers |= Qt::AltModifier;

    if (!isValidBinding())
    {
        mKeyCode = 0;
        mModifiers = Qt::NoModifier;
    }
}

QString KeyBinding::serialValue() const
{
    int mod = 0;
    mod |= mModifiers.testFlag(Qt::ControlModifier) ? 0x01 : 0x00;
    mod |= mModifiers.testFlag(Qt::ShiftModifier) ? 0x02 : 0x00;
    mod |= mModifiers.testFlag(Qt::AltModifier) ? 0x04 : 0x00;
    return QString::number(mKeyCode) + "," + QString::number(mod);
}

} // namespace ctrl
