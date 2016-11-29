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
    ImageFileLoader(const gl::DeviceInfo& aDeviceInfo);

    void setCanvasSize(const QSize& aSize, bool aForce);

    bool load(const QString& aPath, core::Project& aProject,
              util::IProgressReporter& aReporter);

    const QString& log() const { return mLog; }

private:
    bool createEmptyCanvas(core::Project& aProject,
                           const QString& aTopName,
                           const QSize& aCanvasSize);

    bool loadPsd(core::Project& aProject,
                 util::IProgressReporter& aReporter);

    bool loadImage(core::Project& aProject,
                   util::IProgressReporter& aReporter);

    static QRect calculateBoundingRectFromChildren(const core::ObjectNode& aNode);
    void setDefaultPosturesFromInitialRects(core::ObjectNode& aNode);
    bool checkTextureSizeError(uint32 aWidth, uint32 aHeight);

    QString mLog;
    QFileInfo mFileInfo;
    gl::DeviceInfo mGLDeviceInfo;
    QSize mCanvasSize;
    bool mForceCanvasSize;
};

} // namespace ctrl

#endif // CTRL_IMAGEFILELOADER_H
