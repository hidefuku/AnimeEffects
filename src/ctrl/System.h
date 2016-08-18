#ifndef CTRL_SYSTEM_H
#define CTRL_SYSTEM_H

#include <QString>
#include "util/NonCopyable.h"
#include "util/IProgressReporter.h"
#include "gl/DeviceInfo.h"
#include "core/Project.h"

namespace ctrl
{

class System : private util::NonCopyable
{
public:
    System(const QString& aResourceDir, const QString& aCacheDir);
    ~System();

    void setAnimator(core::Animator& aAnimator);
    void setGLDeviceInfo(const gl::DeviceInfo& aInfo);

    core::Project* newProject(
            const QString& aFileName,
            const core::Project::Attribute& aAttr,
            core::Project::Hook* aHookGrabbed,
            util::IProgressReporter& aReporter);

    core::Project* openProject(
            const QString& aFileName,
            core::Project::Hook* aHookGrabbed,
            util::IProgressReporter& aReporter);

    bool saveProject(core::Project& aProject);

    bool closeProject(core::Project& aProject);

    void closeAllProjects();

    bool hasProject() const { return !mProjects.isEmpty(); }
    int projectCount() const { return mProjects.count(); }

    core::Project* project(int aIndex);
    const core::Project* project(int aIndex) const;

private:
    static bool makeSureCacheDirectory(const QString& aCacheDir);
    static bool safeRename(const QString& aSrc, const QString& aDst);

    const QString mResourceDir;
    const QString mCacheDir;
    QVector<core::Project*> mProjects;
    core::Animator* mAnimator;
    gl::DeviceInfo mGLDeviceInfo;
};

} // namespace ctrl

#endif // CTRL_SYSTEM_H
