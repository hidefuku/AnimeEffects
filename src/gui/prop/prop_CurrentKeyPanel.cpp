#include "cmnd/ScopedMacro.h"
#include "cmnd/BasicCommands.h"
#include "core/Constant.h"
#include "core/TimeKeyExpans.h"
#include "core/DepthKey.h"
#include "ctrl/TimeLineUtil.h"
#include "gui/ResourceDialog.h"
#include "gui/prop/prop_CurrentKeyPanel.h"
#include "gui/prop/prop_Items.h"

namespace gui {
namespace prop {

//-------------------------------------------------------------------------------------------------
CurrentKeyPanel::MovePanel::MovePanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
    : mAccessor(aAccessor)
    , mKnocker()
    , mGroup()
    , mEasing()
    , mSpline()
    , mMove()
    , mKeyExists(false)
{
    mKnocker = new KeyKnocker("Move");
    mKnocker->set([=](){ this->mAccessor.knockNewMove(); });
    aPanel.addGroup(mKnocker);

    mGroup = new KeyGroup("Move", aLabelWidth);
    {
        aPanel.addGroup(mGroup);

        // easing
        mEasing = new EasingItem(mGroup);
        mGroup->addItem("easing :", mEasing);
        mEasing->onValueUpdated = [=](util::Easing::Param, util::Easing::Param aNext)
        {
            this->mAccessor.assignMoveEasing(aNext);
        };

        // spline
        mSpline = new ComboItem(mGroup);
        mSpline->box().addItems(QStringList() << "Linear" << "CatmullRom");
        mSpline->setValue(core::MoveKey::kDefaultSplineType, false);
        mSpline->onValueUpdated = [=](int, int aNext)
        {
            this->mAccessor.assignMoveSpline(aNext);
        };
        mGroup->addItem("spline :", mSpline);

        // move
        mMove = new Vector2DItem(mGroup);
        mMove->setRange(core::Constant::transMin(), core::Constant::transMax());
        mMove->onValueUpdated = [=](QVector2D, QVector2D aNext)
        {
            this->mAccessor.assignMovePosition(aNext);
        };
        mGroup->addItem("position :", mMove);
    }
    setEnabled(false);
    setKeyExists(false);
}

void CurrentKeyPanel::MovePanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void CurrentKeyPanel::MovePanel::setKeyExists(bool aIsExists)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists);
    mGroup->setVisible(aIsExists);
}

void CurrentKeyPanel::MovePanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, Move);
    const core::MoveKey::Data data = ((const core::MoveKey*)aKey)->data();
    mEasing->setValue(data.easing(), false);
    mSpline->setValue(data.spline(), false);
    mMove->setValue(data.pos());
}

bool CurrentKeyPanel::MovePanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
CurrentKeyPanel::RotatePanel::RotatePanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
    : mAccessor(aAccessor)
    , mKnocker()
    , mGroup()
    , mEasing()
    , mRotate()
    , mKeyExists(false)
{
    mKnocker = new KeyKnocker("Rotate");
    mKnocker->set([=](){ this->mAccessor.knockNewRotate(); });
    aPanel.addGroup(mKnocker);

    mGroup = new KeyGroup("Rotate", aLabelWidth);
    {
        aPanel.addGroup(mGroup);

        // easing
        mEasing = new EasingItem(mGroup);
        mGroup->addItem("easing :", mEasing);
        mEasing->onValueUpdated = [=](util::Easing::Param, util::Easing::Param aNext)
        {
            this->mAccessor.assignRotateEasing(aNext);
        };

        // rotate
        mRotate = new DecimalItem(mGroup);
        mRotate->setRange(core::Constant::rotateMin(), core::Constant::rotateMax());
        mRotate->onValueUpdated = [=](double, double aNext)
        {
            this->mAccessor.assignRotateAngle(aNext);
        };
        mGroup->addItem("angle :", mRotate);
    }
    setEnabled(false);
    setKeyExists(false);
}

void CurrentKeyPanel::RotatePanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void CurrentKeyPanel::RotatePanel::setKeyExists(bool aIsExists)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists);
    mGroup->setVisible(aIsExists);
}

void CurrentKeyPanel::RotatePanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, Rotate);
    const core::RotateKey::Data data = ((const core::RotateKey*)aKey)->data();
    mEasing->setValue(data.easing(), false);
    mRotate->setValue(data.rotate());
}

bool CurrentKeyPanel::RotatePanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
CurrentKeyPanel::ScalePanel::ScalePanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
    : mAccessor(aAccessor)
    , mKnocker()
    , mGroup()
    , mEasing()
    , mScale()
    , mKeyExists(false)
{
    mKnocker = new KeyKnocker("Scale");
    mKnocker->set([=](){ this->mAccessor.knockNewScale(); });
    aPanel.addGroup(mKnocker);

    mGroup = new KeyGroup("Scale", aLabelWidth);
    {
        aPanel.addGroup(mGroup);

        // easing
        mEasing = new EasingItem(mGroup);
        mGroup->addItem("easing :", mEasing);
        mEasing->onValueUpdated = [=](util::Easing::Param, util::Easing::Param aNext)
        {
            this->mAccessor.assignScaleEasing(aNext);
        };

        // scale
        mScale = new Vector2DItem(mGroup);
        mScale->setRange(core::Constant::scaleMin(), core::Constant::scaleMax());
        mScale->onValueUpdated = [=](QVector2D, QVector2D aNext)
        {
            this->mAccessor.assignScaleRate(aNext);
        };
        mGroup->addItem("rate :", mScale);
    }
    setEnabled(false);
    setKeyExists(false);
}

void CurrentKeyPanel::ScalePanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void CurrentKeyPanel::ScalePanel::setKeyExists(bool aIsExists)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists);
    mGroup->setVisible(aIsExists);
}

void CurrentKeyPanel::ScalePanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, Scale);
    const core::ScaleKey::Data data = ((const core::ScaleKey*)aKey)->data();
    mEasing->setValue(data.easing(), false);
    mScale->setValue(data.scale());
}

bool CurrentKeyPanel::ScalePanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
CurrentKeyPanel::DepthPanel::DepthPanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
    : mAccessor(aAccessor)
    , mKnocker()
    , mGroup()
    , mEasing()
    , mDepth()
    , mKeyExists(false)
{
    mKnocker = new KeyKnocker("Depth");
    mKnocker->set([=](){ this->mAccessor.knockNewDepth(); });
    aPanel.addGroup(mKnocker);

    mGroup = new KeyGroup("Depth", aLabelWidth);
    {
        aPanel.addGroup(mGroup);

        // easing
        mEasing = new EasingItem(mGroup);
        mGroup->addItem("easing :", mEasing);
        mEasing->onValueUpdated = [=](util::Easing::Param, util::Easing::Param aNext)
        {
            this->mAccessor.assignDepthEasing(aNext);
        };

        // depth
        mDepth = new DecimalItem(mGroup);
        mDepth->setRange(core::Constant::transMin(), core::Constant::transMax());
        mDepth->onValueUpdated = [=](double, double aNext)
        {
            this->mAccessor.assignDepthPosition(aNext);
        };
        mGroup->addItem("position :", mDepth);
    }
    setEnabled(false);
    setKeyExists(false);
}

void CurrentKeyPanel::DepthPanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void CurrentKeyPanel::DepthPanel::setKeyExists(bool aIsExists)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists);
    mGroup->setVisible(aIsExists);
}

void CurrentKeyPanel::DepthPanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, Depth);
    const core::DepthKey::Data& data = ((const core::DepthKey*)aKey)->data();
    mEasing->setValue(data.easing(), false);
    mDepth->setValue(data.depth());
}

bool CurrentKeyPanel::DepthPanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
CurrentKeyPanel::OpaPanel::OpaPanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
    : mAccessor(aAccessor)
    , mKnocker()
    , mGroup()
    , mEasing()
    , mOpacity()
    , mKeyExists(false)
{
    mKnocker = new KeyKnocker("Opacity");
    mKnocker->set([=](){ this->mAccessor.knockNewOpacity(); });
    aPanel.addGroup(mKnocker);

    mGroup = new KeyGroup("Opacity", aLabelWidth);
    {
        aPanel.addGroup(mGroup);

        // easing
        mEasing = new EasingItem(mGroup);
        mGroup->addItem("easing :", mEasing);
        mEasing->onValueUpdated = [=](util::Easing::Param, util::Easing::Param aNext)
        {
            this->mAccessor.assignOpaEasing(aNext);
        };

        // opacity
        mOpacity = new DecimalItem(mGroup);
        mOpacity->setRange(0.0f, 1.0f);
        mOpacity->box().setSingleStep(0.1);
        mOpacity->onValueUpdated = [=](double, double aNext)
        {
            this->mAccessor.assignOpacity(aNext);
        };
        mGroup->addItem("rate :", mOpacity);
    }
    setEnabled(false);
    setKeyExists(false);
}

void CurrentKeyPanel::OpaPanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void CurrentKeyPanel::OpaPanel::setKeyExists(bool aIsExists)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists);
    mGroup->setVisible(aIsExists);
}

void CurrentKeyPanel::OpaPanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, Opa);
    const core::OpaKey::Data& data = ((const core::OpaKey*)aKey)->data();
    mEasing->setValue(data.easing(), false);
    mOpacity->setValue(data.opacity());
}

bool CurrentKeyPanel::OpaPanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
CurrentKeyPanel::PosePanel::PosePanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
    : mAccessor(aAccessor)
    , mKnocker()
    , mGroup()
    , mEasing()
    , mKeyExists(false)
{
    mKnocker = new KeyKnocker("Pose");
    mKnocker->set([=](){ this->mAccessor.knockNewPose(); });
    aPanel.addGroup(mKnocker);

    mGroup = new KeyGroup("Pose", aLabelWidth);
    {
        aPanel.addGroup(mGroup);

        // easing
        mEasing = new EasingItem(mGroup);
        mGroup->addItem("easing :", mEasing);
        mEasing->onValueUpdated = [=](util::Easing::Param, util::Easing::Param aNext)
        {
            this->mAccessor.assignPoseEasing(aNext);
        };
    }
    setEnabled(false);
    setKeyExists(false, false);
}

void CurrentKeyPanel::PosePanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void CurrentKeyPanel::PosePanel::setKeyExists(bool aIsExists, bool aIsKnockable)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists && aIsKnockable);
    mGroup->setVisible(aIsExists);
}

void CurrentKeyPanel::PosePanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, Pose);
    const core::PoseKey::Data& data = ((const core::PoseKey*)aKey)->data();
    mEasing->setValue(data.easing(), false);
}

bool CurrentKeyPanel::PosePanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
CurrentKeyPanel::FFDPanel::FFDPanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
    : mAccessor(aAccessor)
    , mKnocker()
    , mGroup()
    , mEasing()
    , mKeyExists(false)
{
    mKnocker = new KeyKnocker("FFD");
    mKnocker->set([=](){ this->mAccessor.knockNewFFD(); });
    aPanel.addGroup(mKnocker);

    mGroup = new KeyGroup("FFD", aLabelWidth);
    {
        aPanel.addGroup(mGroup);

        // easing
        mEasing = new EasingItem(mGroup);
        mGroup->addItem("easing :", mEasing);
        mEasing->onValueUpdated = [=](util::Easing::Param, util::Easing::Param aNext)
        {
            this->mAccessor.assignFFDEasing(aNext);
        };
    }
    setEnabled(false);
    setKeyExists(false, false);
}

void CurrentKeyPanel::FFDPanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void CurrentKeyPanel::FFDPanel::setKeyExists(bool aIsExists, bool aIsKnockable)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists && aIsKnockable);
    mGroup->setVisible(aIsExists);
}

void CurrentKeyPanel::FFDPanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, FFD);
    const core::FFDKey::Data& data = ((const core::FFDKey*)aKey)->data();
    mEasing->setValue(data.easing(), false);
}

bool CurrentKeyPanel::FFDPanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
CurrentKeyPanel::ImagePanel::ImagePanel(Panel& aPanel, KeyAccessor& aAccessor,
                                    int aLabelWidth, ViaPoint& aViaPoint)
    : mAccessor(aAccessor)
    , mKnocker()
    , mGroup()
    , mBrowse()
    , mOffset()
    , mCellSize()
    , mKeyExists(false)
    , mViaPoint(aViaPoint)
{
    mKnocker = new KeyKnocker("Image");
    mKnocker->set([=]() { this->knockNewKey(); });
    aPanel.addGroup(mKnocker);

    mGroup = new KeyGroup("Image", aLabelWidth);
    {
        aPanel.addGroup(mGroup);

        // browse
        mBrowse = new BrowseItem(mGroup);
        mGroup->addItem("resource :", mBrowse);
        mBrowse->onButtonPushed = [=]()
        {
            auto resNode = this->mViaPoint.requireOneResource();
            if (resNode)
            {
                this->mBrowse->setValue(resNode->data().identifier());
                this->mAccessor.assignImageResource(*resNode);
            }
        };
        // offset
        mOffset = new Vector2DItem(mGroup);
        mOffset->setRange(core::Constant::transMin(), core::Constant::transMax());
        mOffset->onValueUpdated = [=](QVector2D, QVector2D aNext)
        {
            this->mAccessor.assignImageOffset(-aNext);
        };
        mGroup->addItem("center :", mOffset);

        // cell size
        mCellSize = new IntegerItem(mGroup);
        mCellSize->setRange(core::Constant::imageCellSizeMin(),
                            core::Constant::imageCellSizeMax());
        mCellSize->onValueUpdated = [=](int, int aNext)
        {
            this->mAccessor.assignImageCellSize(aNext);
        };
        mGroup->addItem("mesh cell :", mCellSize);
    }
    setEnabled(false);
    setKeyExists(false, false);
}

void CurrentKeyPanel::ImagePanel::knockNewKey()
{
    auto resNode = mViaPoint.requireOneResource();
    if (resNode)
    {
        this->mAccessor.knockNewImage(resNode->handle());
    }
}

void CurrentKeyPanel::ImagePanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void CurrentKeyPanel::ImagePanel::setKeyExists(bool aIsExists, bool aIsKnockable)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists && aIsKnockable);
    mGroup->setVisible(aIsExists);
}

void CurrentKeyPanel::ImagePanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, Image);
    const core::ImageKey::Data& data = ((const core::ImageKey*)aKey)->data();
    mBrowse->setValue(data.resource()->identifier());
    mOffset->setValue(-data.imageOffset());
    mCellSize->setValue(data.gridMesh().cellSize());
}

bool CurrentKeyPanel::ImagePanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
CurrentKeyPanel::CurrentKeyPanel(ViaPoint& aViaPoint, core::Project& aProject, const QString& aTitle, QWidget* aParent)
    : Panel(aTitle, aParent)
    , mViaPoint(aViaPoint)
    , mProject(aProject)
    , mTarget()
    , mKeyAccessor()
    , mLabelWidth()
    , mMovePanel()
    , mRotatePanel()
    , mScalePanel()
    , mDepthPanel()
    , mOpaPanel()
    , mPosePanel()
    , mFFDPanel()
    , mImagePanel()
{
    mKeyAccessor.setProject(&aProject);
    mLabelWidth = this->fontMetrics().boundingRect("MaxTextWidth :").width();

    build();
    this->hide();
}

void CurrentKeyPanel::setTarget(core::ObjectNode* aTarget)
{
    mTarget = aTarget;
    mKeyAccessor.setTarget(aTarget);

    if (mTarget)
    {
        this->setTitle(mTarget->name() + " Current Keys");
        this->show();
    }
    else
    {
        this->hide();
    }

    updateKey();
}

void CurrentKeyPanel::setPlayBackActivity(bool aIsActive)
{
    // resume
    if (!this->isEnabled() && !aIsActive)
    {
        updateKeyExists();
        updateKeyValue();
    }
    this->setEnabled(!aIsActive);
}

void CurrentKeyPanel::updateKey()
{
    updateKeyExists();
    updateKeyValue();
}

void CurrentKeyPanel::updateFrame()
{
    if (this->isEnabled())
    {
        updateKeyExists();
        updateKeyValue();
    }
}

void CurrentKeyPanel::build()
{
    using core::Constant;

    mMovePanel.reset(new MovePanel(*this, mKeyAccessor, mLabelWidth));
    mRotatePanel.reset(new RotatePanel(*this, mKeyAccessor, mLabelWidth));
    mScalePanel.reset(new ScalePanel(*this, mKeyAccessor, mLabelWidth));
    mDepthPanel.reset(new DepthPanel(*this, mKeyAccessor, mLabelWidth));
    mOpaPanel.reset(new OpaPanel(*this, mKeyAccessor, mLabelWidth));
    mPosePanel.reset(new PosePanel(*this, mKeyAccessor, mLabelWidth));
    mFFDPanel.reset(new FFDPanel(*this, mKeyAccessor, mLabelWidth));
    mImagePanel.reset(new ImagePanel(*this, mKeyAccessor, mLabelWidth, mViaPoint));

    this->addStretch();
}

void CurrentKeyPanel::updateKeyExists()
{
    if (mTarget && mTarget->timeLine())
    {
        const core::TimeLine& timeLine = *mTarget->timeLine();
        const int frame = mProject.animator().currentFrame().get();
        const bool hasAreaBone = timeLine.current().bone().areaKey();
        const bool hasAnyMesh = mTarget->hasAnyMesh();
        const bool hasAnyImage = mTarget->hasAnyImage();

        mMovePanel->setEnabled(true);
        mMovePanel->setKeyExists(timeLine.hasTimeKey(core::TimeKeyType_Move, frame));
        mRotatePanel->setEnabled(true);
        mRotatePanel->setKeyExists(timeLine.hasTimeKey(core::TimeKeyType_Rotate, frame));
        mScalePanel->setEnabled(true);
        mScalePanel->setKeyExists(timeLine.hasTimeKey(core::TimeKeyType_Scale, frame));
        mDepthPanel->setEnabled(true);
        mDepthPanel->setKeyExists(timeLine.hasTimeKey(core::TimeKeyType_Depth, frame));
        mOpaPanel->setEnabled(true);
        mOpaPanel->setKeyExists(timeLine.hasTimeKey(core::TimeKeyType_Opa, frame));
        mPosePanel->setEnabled(true);
        mPosePanel->setKeyExists(timeLine.hasTimeKey(core::TimeKeyType_Pose, frame), hasAreaBone);
        mFFDPanel->setEnabled(true);
        mFFDPanel->setKeyExists(timeLine.hasTimeKey(core::TimeKeyType_FFD, frame), hasAnyMesh);
        mImagePanel->setEnabled(true);
        mImagePanel->setKeyExists(timeLine.hasTimeKey(core::TimeKeyType_Image, frame), hasAnyImage);
    }
    else
    {
        mMovePanel->setEnabled(false);
        mRotatePanel->setEnabled(false);
        mScalePanel->setEnabled(false);
        mDepthPanel->setEnabled(false);
        mOpaPanel->setEnabled(false);
        mPosePanel->setEnabled(false);
        mFFDPanel->setEnabled(false);
        mImagePanel->setEnabled(false);
    }
}

void CurrentKeyPanel::updateKeyValue()
{
    if (mTarget && mTarget->timeLine())
    {
        auto& timeLine = *mTarget->timeLine();
        const int frame = mProject.animator().currentFrame().get();

        if (mMovePanel->keyExists())
        {
            mMovePanel->setKeyValue(timeLine.timeKey(core::TimeKeyType_Move, frame));
        }
        if (mRotatePanel->keyExists())
        {
            mRotatePanel->setKeyValue(timeLine.timeKey(core::TimeKeyType_Rotate, frame));
        }
        if (mScalePanel->keyExists())
        {
            mScalePanel->setKeyValue(timeLine.timeKey(core::TimeKeyType_Scale, frame));
        }
        if (mDepthPanel->keyExists())
        {
            mDepthPanel->setKeyValue(timeLine.timeKey(core::TimeKeyType_Depth, frame));
        }
        if (mOpaPanel->keyExists())
        {
            mOpaPanel->setKeyValue(timeLine.timeKey(core::TimeKeyType_Opa, frame));
        }
        if (mPosePanel->keyExists())
        {
            mPosePanel->setKeyValue(timeLine.timeKey(core::TimeKeyType_Pose, frame));
        }
        if (mFFDPanel->keyExists())
        {
            mFFDPanel->setKeyValue(timeLine.timeKey(core::TimeKeyType_FFD, frame));
        }
        if (mImagePanel->keyExists())
        {
            mImagePanel->setKeyValue(timeLine.timeKey(core::TimeKeyType_Image, frame));
        }
    }
}

} // namespace prop
} // namespace gui

