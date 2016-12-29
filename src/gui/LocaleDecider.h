#ifndef GUI_LOCALEDECIDER_H
#define GUI_LOCALEDECIDER_H

#include <QString>
#include <QTranslator>

class LocaleDecider
{
public:
    LocaleDecider();
    const QString& preferFont() const { return mPreferFont; }
    QTranslator* translator() { return mHasTranslator ? &mTranslator : nullptr; }

private:
    QString mPreferFont;
    QTranslator mTranslator;
    bool mHasTranslator;
};

#endif // GUI_LOCALEDECIDER_H
