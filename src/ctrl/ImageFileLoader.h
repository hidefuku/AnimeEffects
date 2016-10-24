#ifndef CTRL_IMAGEFILELOADER_H
#define CTRL_IMAGEFILELOADER_H

#include <QFileInfo>
#include <QString>
#include <QScopedPointer>
#include "util/IProgressReporter.h"
#include "gl/DeviceInfo.h"
#include "core/Project.h"
#include "core/ObjectTree.h"

namespace ctrl
{

class ImageFileLoader
{
public:
    ImageFileLoader();
    bool load(const QFileInfo& aPath, core::Project& aProject,
              const gl::DeviceInfo& aGLDeviceInfo,
              util::IProgressReporter& aReporter);

    const QString& log() const { return mLog; }

private:
    bool loadPsd(
            core::Project& aProject,
            util::IProgressReporter& aReporter);

    static QRect calculateBoundingRectFromChildren(const core::ObjectNode& aNode);
    void setDefaultPositions(core::ObjectNode& aNode);
    bool checkTextureSizeError(uint32 aWidth, uint32 aHeight);

    QString mLog;
    QScopedPointer<std::ifstream> mFile;
    QFileInfo mFileInfo;
    gl::DeviceInfo mGLDeviceInfo;
};

} // namespace ctrl

#endif // CTRL_IMAGEFILELOADER_H
