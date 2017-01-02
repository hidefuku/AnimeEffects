#include "gui/ViaPoint.h"
#include "gui/ResourceDialog.h"
#include "gui/KeyCommandInvoker.h"

namespace gui
{

ViaPoint::ViaPoint(QWidget* aParent)
    : mParent(aParent)
    , mProject()
    , mResDialog()
    , mLogView()
    , mGLDeviceInfo()
    , mKeyCommandMap()
    , mKeyCommandInvoker()
    , mMainViewSetting()
{
}

void ViaPoint::setProject(core::Project* aProject)
{
    mProject = aProject;
}

void ViaPoint::setMainMenuBar(MainMenuBar* aMainMenuBar)
{
    mMainMenuBar = aMainMenuBar;
}

void ViaPoint::setResourceDialog(ResourceDialog* aResDialog)
{
    mResDialog = aResDialog;
}

img::ResourceNode* ViaPoint::requireOneResource()
{
    QScopedPointer<ResourceDialog> dialog(new ResourceDialog(*this, true, mParent));
    dialog->setProject(mProject);
    dialog->exec();

    if (dialog->hasValidNode())
    {
        return dialog->nodeList().first();
    }
    return nullptr;
}

void ViaPoint::setLogView(QPlainTextEdit* aLogView)
{
    mLogView = aLogView;
}

void ViaPoint::pushLog(const QString& aText, ctrl::UILogType aType)
{
    (void)aType;
    if (mLogView)
    {
        mLogView->appendPlainText(aText);
    }
}

void ViaPoint::pushUndoneLog(const QString& aText)
{
    if (mLogView)
    {
        mLogView->appendHtml("<font color=\"#606060\">" + aText + "</font>");
    }
}

void ViaPoint::pushRedoneLog(const QString& aText)
{
    if (mLogView)
    {
        mLogView->appendHtml("<font color=\"#000030\">" + aText + "</font>");
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

void ViaPoint::setKeyCommandMap(KeyCommandMap* aMap)
{
    mKeyCommandMap = aMap;
}

void ViaPoint::setKeyCommandInvoker(KeyCommandInvoker* aInvoker)
{
    mKeyCommandInvoker = aInvoker;
}

void ViaPoint::throwKeyPressingToKeyCommandInvoker(const QKeyEvent* aEvent)
{
    if (mKeyCommandInvoker)
    {
        mKeyCommandInvoker->onKeyPressed(aEvent);
    }
}

void ViaPoint::throwKeyReleasingToKeyCommandInvoker(const QKeyEvent* aEvent)
{
    if (mKeyCommandInvoker)
    {
        mKeyCommandInvoker->onKeyReleased(aEvent);
    }
}

void ViaPoint::setMainViewSetting(MainViewSetting& aSetting)
{
    mMainViewSetting = &aSetting;
}

MainViewSetting& ViaPoint::mainViewSetting()
{
    XC_PTR_ASSERT(mMainViewSetting);
    return *mMainViewSetting;
}

const MainViewSetting& ViaPoint::mainViewSetting() const
{
    XC_PTR_ASSERT(mMainViewSetting);
    return *mMainViewSetting;
}

} // namespace gui

