#include <QDir>
#include <QFile>
#include "gl/Global.h"
#include "ctrl/System.h"
#include "ctrl/ProjectSaver.h"
#include "ctrl/ProjectLoader.h"
#include "ctrl/ImageFileLoader.h"

using namespace core;

namespace
{
static const int kStandardFps = 60;
}

namespace ctrl
{

System::System(const QString& aResourceDir, const QString& aCacheDir)
    : mResourceDir(aResourceDir)
    , mCacheDir(aCacheDir)
    , mProjects()
    , mAnimator()
    , mGLDeviceInfo()
{
}

System::~System()
{
    closeAllProjects();
}

void System::setAnimator(Animator& aAnimator)
{
    mAnimator = &aAnimator;
}

void System::setGLDeviceInfo(const gl::DeviceInfo& aInfo)
{
    mGLDeviceInfo = aInfo;
}

core::Project* System::newProject(
        const QString& aFileName,
        const core::Project::Attribute& aAttr,
        core::Project::Hook* aHookGrabbed,
        util::IProgressReporter& aReporter)
{
    QScopedPointer<core::Project::Hook> hookScope(aHookGrabbed);

    XC_ASSERT(mAnimator);
    gl::Global::makeCurrent();

    QScopedPointer<core::Project> projectScope;
    projectScope.reset(new Project(QString(), *mAnimator, hookScope.take()));
    projectScope->attribute() = aAttr;
    projectScope->resourceHolder().setRootPath(QFileInfo(aFileName).path());

    ctrl::ImageFileLoader loader;
    if (loader.load(QFileInfo(aFileName), *projectScope, mGLDeviceInfo, aReporter))
    {
        mProjects.push_back(projectScope.take());
        return mProjects.back();
    }

    qDebug() << "failed to load image : " << loader.log();
    return nullptr;
}

core::Project* System::openProject(
        const QString& aFileName,
        Project::Hook* aHookGrabbed,
        util::IProgressReporter& aReporter)
{
    QScopedPointer<core::Project::Hook> hookScope(aHookGrabbed);

    XC_ASSERT(mAnimator);
    XC_ASSERT(mGLDeviceInfo.isValid());

    gl::Global::makeCurrent();

    if (!aFileName.isEmpty())
    {
        QScopedPointer<core::Project> projectScope;
        projectScope.reset(new Project(aFileName, *mAnimator, hookScope.take()));

        ctrl::ProjectLoader loader;
        if (loader.load(aFileName, *projectScope, mGLDeviceInfo, aReporter))
        {
            mProjects.push_back(projectScope.take());
            return mProjects.back();
        }
        for (auto log : loader.log())
        {
            qDebug() << log;
        }
    }

    return nullptr;
}

bool System::saveProject(core::Project& aProject)
{
    const int index = mProjects.indexOf(&aProject);

    if (index < 0 || mProjects.count() <= index)
    {
        return false;
    }

    auto project = mProjects.at(index);

    if (project && !project->isNameless())
    {
        const QString outputPath = project->fileName();
        const QString cachePath = mCacheDir + "/lastproject.cache";

        // create cache directory
        if (!makeSureCacheDirectory(mCacheDir))
        {
            qDebug() << "failed to create cache directory.";
            return false;
        }

        ctrl::ProjectSaver saver;

        if (!saver.save(cachePath, *project))
        {
            qDebug() << "failed to save a project.\n" << outputPath;
            qDebug() << saver.log();
            return false;
        }

        if (!safeRename(cachePath, outputPath))
        {
            qDebug() << "failed to rename a project file.\n" << outputPath;
            return false;
        }

        project->commandStack().resetEditingOrigin();
        qDebug() << "save a project file. " << outputPath;
        return true;
    }
    return false;
}

bool System::closeProject(core::Project& aProject)
{
    const int index = mProjects.indexOf(&aProject);

    if (0 <= index && index < mProjects.count())
    {
        auto ptr = mProjects.at(index);
        mProjects.removeAt(index);
        delete ptr;
        return true;
    }
    return false;
}

void System::closeAllProjects()
{
    qDeleteAll(mProjects);
    mProjects.clear();
}

core::Project* System::project(int aIndex)
{
    XC_ASSERT(0 <= aIndex && aIndex < mProjects.count());
    return mProjects[aIndex];
}

const core::Project* System::project(int aIndex) const
{
    XC_ASSERT(0 <= aIndex && aIndex < mProjects.count());
    return mProjects[aIndex];
}

bool System::makeSureCacheDirectory(const QString& aCacheDir)
{
    if (!QDir::current().exists(aCacheDir))
    {
        if (!QDir::current().mkpath(aCacheDir))
        {
            qDebug() << "failed to create cache directory.";
            return false;
        }
    }
    return true;
}

bool System::safeRename(const QString& aSrc, const QString& aDst)
{
    if (!QFile::exists(aSrc))
    {
        qDebug() << "failed to find a project cache file.";
        return false;
    }

    if (QFile::exists(aDst))
    {
        if (!QFile::remove(aDst))
        {
            qDebug() << "failed to remove a project file.";
            return false;
        }
    }

    if (!QFile::rename(aSrc, aDst))
    {
        qDebug() << "failed to rename a project file.";
        return false;
    }
    return true;
}

} // namespace ctrl

