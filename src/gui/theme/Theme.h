#ifndef GUI_THEME_INFO_H
#define GUI_THEME_INFO_H

#include <QString>
#include <QFileInfo>

#include <QPalette>

namespace theme
{

class Theme
{

public:
    Theme();
    Theme(QString aResourceDir);
    Theme(QString aResourceDir, QString aId);

    QString id() const;
    QString path() const;
    QFileInfo fileInfo() const;
    bool isDefault() const;

    bool isDark() const;

private:
    QString mId;
    QFileInfo mFileInfo;
};

} // namespace theme

#endif // GUI_THEME_INFO_H
