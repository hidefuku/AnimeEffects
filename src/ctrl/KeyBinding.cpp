#include <QKeySequence>
#include "ctrl/KeyBinding.h"

namespace ctrl
{

//-------------------------------------------------------------------------------------------------
bool KeyBinding::getKeyValidity(int aKeyCode)
{
    if (Qt::Key_Space <= aKeyCode && aKeyCode <= Qt::Key_AsciiTilde)
    {
        return true;
    }

    if (Qt::Key_F1 <= aKeyCode && aKeyCode <= Qt::Key_F35)
    {
        return true;
    }

    switch (aKeyCode)
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

//-------------------------------------------------------------------------------------------------
KeyBinding::KeyBinding()
    : mKeyCode(-1)
    , mSubKeyCode(-1)
    , mModifiers(Qt::NoModifier)
{
}

KeyBinding::KeyBinding(int aKeyCode, Qt::KeyboardModifiers aModifiers, int aSubKeyCode)
    : mKeyCode(aKeyCode)
    , mSubKeyCode(aSubKeyCode)
    , mModifiers()
{
    mModifiers = aModifiers & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier | Qt::MetaModifier);
}

void KeyBinding::setSubKeyCode(int aSubKeyCode)
{
    mSubKeyCode = getKeyValidity(aSubKeyCode) ? aSubKeyCode : -1;
}

bool KeyBinding::isValidBinding() const
{
    return getKeyValidity(mKeyCode) &&
            (mSubKeyCode == -1 || getKeyValidity(mSubKeyCode));
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

bool KeyBinding::hasMetaModifier() const
{
    return mModifiers & Qt::MetaModifier;
}

bool KeyBinding::hasAnyModifiers() const
{
    return mModifiers != Qt::NoModifier;
}

QString KeyBinding::text() const
{
    QString t;
#if defined (Q_OS_MAC)
    if (hasControlModifier()) t += "Cmnd + ";
    if (hasMetaModifier()) t += "Ctrl + ";
#elif defined(Q_OS_WIN)
    if (hasControlModifier()) t += "Ctrl + ";
    if (hasMetaModifier()) t += "Meta + ";
#else
    if (hasControlModifier()) t += "Ctrl + ";
    if (hasMetaModifier()) t += "Meta + ";
#endif
    if (hasShiftModifier()) t += "Shift + ";
    if (hasAltModifier()) t += "Alt + ";

    if (mKeyCode != -1)
    {
        t += QKeySequence(mKeyCode).toString();
    }
    if (mSubKeyCode != -1)
    {
        t += QString(", ") + QKeySequence(mSubKeyCode).toString();
    }
    return t;
}

bool KeyBinding::conflictsWith(const KeyBinding& aRhs) const
{
    return mKeyCode == aRhs.mKeyCode &&
            mSubKeyCode == aRhs.mSubKeyCode &&
            mModifiers == aRhs.mModifiers;
}

bool KeyBinding::matchesExactlyWith(const KeyBinding& aRhs) const
{
    return mKeyCode == aRhs.mKeyCode &&
            mSubKeyCode == aRhs.mSubKeyCode &&
            mModifiers == aRhs.mModifiers;
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

    int subKey = -1;
    if (v.size() >= 3)
    {
        subKey = v[2].toInt(&success);
        if (!success) return;
    }

    mKeyCode = key;
    mSubKeyCode = subKey;
    mModifiers = Qt::NoModifier;
    if (mod & 0x01) mModifiers |= Qt::ControlModifier;
    if (mod & 0x02) mModifiers |= Qt::ShiftModifier;
    if (mod & 0x04) mModifiers |= Qt::AltModifier;
    if (mod & 0x08) mModifiers |= Qt::MetaModifier;

    if (!isValidBinding())
    {
        mKeyCode = -1;
        mSubKeyCode = -1;
        mModifiers = Qt::NoModifier;
    }
}

QString KeyBinding::serialValue() const
{
    int mod = 0;
    mod |= mModifiers.testFlag(Qt::ControlModifier) ? 0x01 : 0x00;
    mod |= mModifiers.testFlag(Qt::ShiftModifier) ? 0x02 : 0x00;
    mod |= mModifiers.testFlag(Qt::AltModifier) ? 0x04 : 0x00;
    mod |= mModifiers.testFlag(Qt::MetaModifier) ? 0x08 : 0x00;
    return QString::number(mKeyCode) + "," + QString::number(mod) + "," + QString::number(mSubKeyCode);
}

} // namespace ctrl
