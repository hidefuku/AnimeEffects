#include "gui/ViaPoint.h"
#include "gui/ResourceDialog.h"

namespace gui
{

ViaPoint::ViaPoint(QWidget* aParent)
    : mParent(aParent)
    , mProject()
    , mResDialog()
    , mLogView()
    , mGLDeviceInfo()
{
}

void ViaPoint::setProject(core::Project* aProject)
{
    mProject = aProject;

    if (mResDialog)
    {
        mResDialog->setProject(aProject);
        mResDialog->updateResources();
    }
}

void ViaPoint::setMainMenuBar(MainMenuBar* aMainMenuBar)
{
    mMainMenuBar = aMainMenuBar;
}

void ViaPoint::createResourceDialog()
{
    if (!mResDialog)
    {
        mResDialog = new ResourceDialog(*this, false, mParent);

        if (mProject)
        {
            mResDialog->setProject(mProject);
            mResDialog->updateResources();
        }
    }
    mResDialog->show();
}

void ViaPoint::setLogView(QPlainTextEdit* aLogView)
{
    mLogView = aLogView;
}

void ViaPoint::pushLog(const QString& aText)
{
    if (mLogView)
    {
        mLogView->appendPlainText(aText);
    }
}

void ViaPoint::setGLDeviceInfo(const gl::DeviceInfo& aInfo)
{
    mGLDeviceInfo = aInfo;
}

const gl::DeviceInfo& ViaPoint::glDeviceInfo() const
{
    XC_ASSERT(mGLDeviceInfo.isValid());
    return mGLDeviceInfo;
}

} // namespace gui

