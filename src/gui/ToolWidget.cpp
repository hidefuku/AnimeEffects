#include "gui/ToolWidget.h"
#include "XC.h"

namespace gui
{

ToolWidget::ToolWidget(ViaPoint& aViaPoint, GUIResources& aResources,
                       const QSize& aSizeHint, QWidget* aParent)
    : QWidget(aParent)
    , mViaPoint(aViaPoint)
    , mResources(aResources)
    , mSizeHint(aSizeHint)
    , mViewPanel()
    , mModePanel()
    , mDriver()
    , mToolType(ctrl::ToolType_TERM)
    , mSRTPanel()
    , mFFDPanel()
    , mBonePanel()
    , mMeshPanel()
{
    createViewPanel();

    createModePanel();
    setPanelActivity(false);

    mSRTPanel = new tool::SRTPanel(this, mResources);
    mSRTPanel->onParamUpdated.connect(this, &ToolWidget::onParamUpdated);
    mSRTPanel->hide();

    mFFDPanel = new tool::FFDPanel(this, mResources);
    mFFDPanel->onParamUpdated.connect(this, &ToolWidget::onParamUpdated);
    mFFDPanel->hide();

    mBonePanel = new tool::BonePanel(this, mResources);
    mBonePanel->onParamUpdated.connect(this, &ToolWidget::onParamUpdated);
    mBonePanel->hide();

    mMeshPanel = new tool::MeshPanel(this, mResources);
    mMeshPanel->onParamUpdated.connect(this, &ToolWidget::onParamUpdated);
    mMeshPanel->hide();

    updateGeometry();
}

void ToolWidget::setDriver(ctrl::Driver* aDriver)
{
    mDriver = aDriver;

    if (mDriver)
    {
        mDriver->setTool(mToolType);
        setPanelActivity(true);
    }
    else
    {
        setPanelActivity(false);
    }
}

void ToolWidget::createViewPanel()
{
    if (mViewPanel) delete mViewPanel;
    mViewPanel = new tool::ViewPanel(this, mResources);

    mViewPanel->addButton("showmesh", true, "Show Layer Mesh", [=](bool aChecked)
    {
        this->viewSetting().showLayerMesh = aChecked;
        this->onViewSettingChanged(this->viewSetting());
    });
    mViewPanel->addButton("cutimages", true, "Cut Images by the Frame", [=](bool aChecked)
    {
        this->viewSetting().cutImagesByTheFrame = aChecked;
        this->onViewSettingChanged(this->viewSetting());
    });
    mViewPanel->addButton("rotateac", false, "Rotate the View Anticlockwise", [=](bool)
    {
        this->viewSetting().rotateViewACW = true;
        this->onViewSettingChanged(this->viewSetting());
        this->viewSetting().rotateViewACW = false;
    });
    mViewPanel->addButton("resetrot", false, "Reset Rotation of the View", [=](bool)
    {
        this->viewSetting().resetRotateView = true;
        this->onViewSettingChanged(this->viewSetting());
        this->viewSetting().resetRotateView = false;
    });
    mViewPanel->addButton("rotatecw", false, "Rotate the View Clockwise", [=](bool)
    {
        this->viewSetting().rotateViewCW = true;
        this->onViewSettingChanged(this->viewSetting());
        this->viewSetting().rotateViewCW = false;
    });
}

void ToolWidget::createModePanel()
{
    if (mModePanel) delete mModePanel;
    mModePanel = new tool::ModePanel(this, mResources);

    auto delegate = [=](ctrl::ToolType aType, bool aChecked)
    {
        this->onModePanelPushed(aType, aChecked);
    };

    mModePanel->addButton(ctrl::ToolType_Cursor,    "cursor",    delegate, "Camera Cursor");
    mModePanel->addButton(ctrl::ToolType_SRT,       "srt",       delegate, "Scale Rotate Translate");
    mModePanel->addButton(ctrl::ToolType_Bone,      "bone",      delegate, "Bone Creating");
    mModePanel->addButton(ctrl::ToolType_Pose,      "pose",      delegate, "Bone Posing");
    mModePanel->addButton(ctrl::ToolType_Mesh,      "mesh",      delegate, "Mesh Creating");
    mModePanel->addButton(ctrl::ToolType_FFD,       "ffd",       delegate, "Free Form Deform");
}

void ToolWidget::setPanelActivity(bool aIsActive)
{
    if (mModePanel)
    {
        for (int i = 0; i < ctrl::ToolType_TERM; ++i)
        {
            setButtonActivity((ctrl::ToolType)i, aIsActive);
        }
    }
}

void ToolWidget::setButtonActivity(ctrl::ToolType aType, bool aIsActive)
{
    if (mModePanel)
    {
        if (aIsActive)
        {
            mModePanel->button(aType)->setEnabled(true);
        }
        else
        {
            if (mModePanel->button(aType)->isChecked())
            {
                mModePanel->button(aType)->setChecked(false);
            }
            mModePanel->button(aType)->setEnabled(false);
        }
    }
}

void ToolWidget::onModePanelPushed(ctrl::ToolType aType, bool aChecked)
{
    mToolType = aChecked ? aType : ctrl::ToolType_TERM;
    if (mDriver)
    {
        mDriver->setTool(mToolType);
        onToolChanged(mToolType);

        mSRTPanel->hide();
        mFFDPanel->hide();
        mBonePanel->hide();
        mMeshPanel->hide();

        if (mToolType == ctrl::ToolType_SRT)
        {
            mSRTPanel->show();
        }
        else if (mToolType == ctrl::ToolType_Bone)
        {
            mBonePanel->show();
        }
        else if (mToolType == ctrl::ToolType_FFD)
        {
            mFFDPanel->show();
        }
        else if (mToolType == ctrl::ToolType_Mesh)
        {
            mMeshPanel->show();
        }
        onParamUpdated(false);

        onVisualUpdated();
    }
}

void ToolWidget::onParamUpdated(bool aLayoutChanged)
{
    if (mDriver)
    {
        if (aLayoutChanged)
        {
            updateGeometry();
        }

        if (mToolType == ctrl::ToolType_SRT)
        {
            mDriver->updateParam(mSRTPanel->param());
            onVisualUpdated();
        }
        else if (mToolType == ctrl::ToolType_Bone)
        {
            mDriver->updateParam(mBonePanel->param());
            onVisualUpdated();
        }
        else if (mToolType == ctrl::ToolType_FFD)
        {
            mDriver->updateParam(mFFDPanel->param());
            onVisualUpdated();
        }
        else if (mToolType == ctrl::ToolType_Mesh)
        {
            mDriver->updateParam(mMeshPanel->param());
            onVisualUpdated();
        }
    }
}

void ToolWidget::resizeEvent(QResizeEvent* aEvent)
{
    QWidget::resizeEvent(aEvent);
    updateGeometry();
}

void ToolWidget::updateGeometry()
{
    const int width = this->size().width();
    int height = 0;

    height = mViewPanel->updateGeometry(QPoint(3, height), width);

    height = mModePanel->updateGeometry(QPoint(3, height), width);

    mSRTPanel->updateGeometry(QPoint(3, height), width);
    mBonePanel->updateGeometry(QPoint(3, height), width);
    mFFDPanel->updateGeometry(QPoint(3, height), width);
    mMeshPanel->updateGeometry(QPoint(3, height), width);
}

} // namespace gui
