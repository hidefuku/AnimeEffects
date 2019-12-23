#include "gui/theme/Theme.h"

namespace theme
{

Theme::Theme()
    : mId("default")
    , mFileInfo()
{

}

Theme::Theme(QString aResourceDir)
    : mId("default")
    , mFileInfo(aResourceDir+"/themes/"+mId)
{

}

Theme::Theme(QString aResourceDir, QString aId)
    : mId(aId)
    , mFileInfo(aResourceDir+"/themes/"+mId)
{

}

//-------------------------------------------------------------------------------------------------
QString Theme::id() const
{
    return mId;
}

QString Theme::path() const
{
    return mFileInfo.absoluteFilePath();
}

QFileInfo Theme::fileInfo() const
{
    return mFileInfo;
}

bool Theme::isDefault() const
{
    return mId == "default";
}

} // namespace theme
