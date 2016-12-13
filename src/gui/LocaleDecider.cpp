#include <QFile>
#include <QTextStream>
#include "gui/LocaleDecider.h"

LocaleDecider::LocaleDecider(bool aForceDefault)
    : mPreferFont()
    , mTranslator()
    , mHasTranslator()
{
    QString locAbb;

    if (!aForceDefault)
    {
        auto language = QLocale::system().language();
        if (language == QLocale::Japanese)
        {
            locAbb = "ja";
        }
    }

    if (!locAbb.isEmpty())
    {
        mTranslator.load("translation_" + locAbb, "data/locale");
        mHasTranslator = true;
    }

    {
#if defined(Q_OS_WIN)
        const QString opt = "_win";
#elif defined(Q_OS_MAC)
        const QString opt = "_mac";
#else
        const QString opt = "";
#endif

        const QString preference = locAbb.isEmpty() ?
                    "preference" : "preference_" + locAbb;

        QFile file("./data/locale/" + preference + ".txt");
        if (file.open(QIODevice::ReadOnly))
        {
            QTextStream in(&file);
            while (!in.atEnd())
            {
                auto kv = in.readLine().split('=');
                if (kv.count() != 2) continue;
                auto key = kv[0].trimmed();
                auto value = kv[1].trimmed();

                if (key == "font_family" + opt)
                {
                    mPreferFont = value;
                }
            }
        }
    }

}
