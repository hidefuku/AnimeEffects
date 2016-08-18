#ifndef CTRL_PROJECTLOADER_H
#define CTRL_PROJECTLOADER_H

#include <QStringList>
#include "util/StreamReader.h"
#include "util/IProgressReporter.h"
#include "gl/DeviceInfo.h"
#include "core/Project.h"

namespace ctrl
{

class ProjectLoader
{
public:
    ProjectLoader();

    bool load(
            const QString& aPath, core::Project& aProject,
            const gl::DeviceInfo& aGLDeviceInfo,
            util::IProgressReporter& aReporter);

    const QStringList& log() const { return mLog; }

private:
    bool readHeader(util::LEStreamReader& aReader);
    bool readGlobalBlock(util::LEStreamReader& aReader, core::Project& aProject);
    QStringList mLog;
};

} // namespace ctrl

#endif // CTRL_PROJECTLOADER_H
