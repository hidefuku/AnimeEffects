#include <QDir>
#include <QFile>
#include "gl/Global.h"
#include "gl/DeviceInfo.h"
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
//-------------------------------------------------------------------------------------------------
System::SaveResult::SaveResult()
    : success()
    , message()
{
}

System::SaveResult::SaveResult(bool aSuccess, const QString& aMessage)
    : success(aSuccess)
    , message(aMessage)
{
}

//-------------------------------------------------------------------------------------------------
System::LoadResult::LoadResult()
    : project()
    , message()
{
}

System::LoadResult::LoadResult(core::Project* aProject, const QString& aMessage)
    : project(aProject)
    , message(aMessage)
{
}

System::LoadResult::LoadResult(core::Project* aProject, const QStringList& aMessage)
    : project(aProject)
    , message(aMessage)
{
}

QString System::LoadResult::messages() const
{
    QString msgs;
    for (auto it = message.rbegin(); it != message.rend(); ++it)
    {
        msgs += *it + "\n";
    }
    return msgs;
}

//-------------------------------------------------------------------------------------------------
System::System(const QString& aResourceDir, const QString& aCacheDir)
    : mResourceDir(aResourceDir)
    , mCacheDir(aCacheDir)
    , mProjects()
    , mAnimator()
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

System::LoadResult System::newProject(
        const QString& aFileName,
        const core::Project::Attribute& aAttr,
        core::Project::Hook* aHookGrabbed,
        util::IProgressReporter& aReporter,
        bool aSpecifiesCanvasSize)
{
    QScopedPointer<core::Project::Hook> hookScope(aHookGrabbed);

    XC_ASSERT(mAnimator);
    XC_ASSERT(gl::DeviceInfo::validInstanceExists());

    gl::Global::makeCurrent();

    QScopedPointer<core::Project> projectScope;
    projectScope.reset(new Project(QString(), *mAnimator, hookScope.take()));
    projectScope->attribute() = aAttr;
    projectScope->resourceHolder().setRootPath(QFileInfo(aFileName).path());

    ctrl::ImageFileLoader loader(gl::DeviceInfo::instance());
    loader.setCanvasSize(aAttr.imageSize(), aSpecifiesCanvasSize);

    if (loader.load(aFileName, *projectScope, aReporter))
    {
        mProjects.push_back(projectScope.take());
        return LoadResult(mProjects.back(), "Success.");
    }

    LoadResult result;
    result.message.append(loader.log());
    result.message.append("Failed to load the image file.");
    return result;
}

System::LoadResult System::openProject(
        const QString& aFileName,
        Project::Hook* aHookGrabbed,
        util::IProgressReporter& aReporter)
{
    QScopedPointer<core::Project::Hook> hookScope(aHookGrabbed);

    XC_ASSERT(mAnimator);
    XC_ASSERT(gl::DeviceInfo::validInstanceExists());

    gl::Global::makeCurrent();

    if (!aFileName.isEmpty())
    {
        QScopedPointer<core::Project> projectScope;
        projectScope.reset(new Project(aFileName, *mAnimator, hookScope.take()));

        ctrl::ProjectLoader loader;
        if (loader.load(aFileName, *projectScope, gl::DeviceInfo::instance(), aReporter))
        {
            mProjects.push_back(projectScope.take());
            return LoadResult(mProjects.back(), "Success.");
        }

        for (auto log : loader.log())
        {
            qDebug() << log;
        }

        LoadResult result;
        result.message.append(loader.log());
        result.message.append("Failed to load project.");
        return result;
    }

    return LoadResult(nullptr, "Empty file.");
}

System::SaveResult System::saveProject(core::Project& aProject)
{
    const int index = mProjects.indexOf(&aProject);

    if (index < 0 || mProjects.count() <= index)
    {
        return SaveResult(false, "Invalid project reference.");
    }

    auto project = mProjects.at(index);

    if (project && !project->isNameless())
    {
        const QString outputPath = project->fileName();
        const QString cachePath = mCacheDir + "/lastproject.cache";

        // create cache directory
        if (!makeSureCacheDirectory(mCacheDir))
        {
            return SaveResult(false, "Failed to create cache directory.");
        }

        ctrl::ProjectSaver saver;

        if (!saver.save(cachePath, *project))
        {
            return SaveResult(false, "Failed to save project. (" + saver.log() + ")");
        }

        if (!safeRename(cachePath, outputPath))
        {
            return SaveResult(false, "Failed to rename the project file.");
        }

        project->commandStack().resetEditingOrigin();
        qDebug() << "save the project file. " << outputPath;
        return SaveResult(true, "Success.");
    }

    return SaveResult(false, "Invalid operation.");
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

bool System::hasModifiedProject() const
{
    for (auto project : mProjects)
    {
        if (project->isModified()) return true;
    }
    return false;
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

