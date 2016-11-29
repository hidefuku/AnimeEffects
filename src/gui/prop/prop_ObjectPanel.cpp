#include "cmnd/ScopedMacro.h"
#include "cmnd/BasicCommands.h"
#include "core/Constant.h"
#include "core/TimeKeyExpans.h"
#include "ctrl/TimeLineUtil.h"
#include "gui/ResourceDialog.h"
#include "gui/prop/prop_ObjectPanel.h"
#include "gui/prop/prop_Items.h"

namespace
{
class ObjectNodeAttrNotifier : public cmnd::Listener
{
public:
    ObjectNodeAttrNotifier(core::Project& aProject, core::ObjectNode& aNode)
        : mProject(aProject)
        , mNode(aNode)
    {}
    virtual void onExecuted() { mProject.onNodeAttributeModified(mNode, false); }
    virtual void onUndone() { mProject.onNodeAttributeModified(mNode, true); }
    virtual void onRedone() { mProject.onNodeAttributeModified(mNode, false); }

private:
    core::Project& mProject;
    core::ObjectNode& mNode;
};

}

namespace gui {
namespace prop {

//-------------------------------------------------------------------------------------------------
ObjectPanel::MovePanel::MovePanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
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

void ObjectPanel::MovePanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void ObjectPanel::MovePanel::setKeyExists(bool aIsExists)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists);
    mGroup->setVisible(aIsExists);
}

void ObjectPanel::MovePanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, Move);
    const core::MoveKey::Data data = ((const core::MoveKey*)aKey)->data();
    mEasing->setValue(data.easing(), false);
    mSpline->setValue(data.spline(), false);
    mMove->setValue(data.pos());
}

bool ObjectPanel::MovePanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
ObjectPanel::RotatePanel::RotatePanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
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

void ObjectPanel::RotatePanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void ObjectPanel::RotatePanel::setKeyExists(bool aIsExists)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists);
    mGroup->setVisible(aIsExists);
}

void ObjectPanel::RotatePanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, Rotate);
    const core::RotateKey::Data data = ((const core::RotateKey*)aKey)->data();
    mEasing->setValue(data.easing(), false);
    mRotate->setValue(data.rotate());
}

bool ObjectPanel::RotatePanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
ObjectPanel::ScalePanel::ScalePanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
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

void ObjectPanel::ScalePanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void ObjectPanel::ScalePanel::setKeyExists(bool aIsExists)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists);
    mGroup->setVisible(aIsExists);
}

void ObjectPanel::ScalePanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, Scale);
    const core::ScaleKey::Data data = ((const core::ScaleKey*)aKey)->data();
    mEasing->setValue(data.easing(), false);
    mScale->setValue(data.scale());
}

bool ObjectPanel::ScalePanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
ObjectPanel::OpaPanel::OpaPanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
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
        mGroup->addItem("opacity :", mOpacity);
    }
    setEnabled(false);
    setKeyExists(false);
}

void ObjectPanel::OpaPanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void ObjectPanel::OpaPanel::setKeyExists(bool aIsExists)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists);
    mGroup->setVisible(aIsExists);
}

void ObjectPanel::OpaPanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, Opa);
    const core::OpaKey::Data& data = ((const core::OpaKey*)aKey)->data();
    mEasing->setValue(data.easing(), false);
    mOpacity->setValue(data.opacity());
}

bool ObjectPanel::OpaPanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
ObjectPanel::PosePanel::PosePanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
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

void ObjectPanel::PosePanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void ObjectPanel::PosePanel::setKeyExists(bool aIsExists, bool aIsKnockable)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists && aIsKnockable);
    mGroup->setVisible(aIsExists);
}

void ObjectPanel::PosePanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, Pose);
    const core::PoseKey::Data& data = ((const core::PoseKey*)aKey)->data();
    mEasing->setValue(data.easing(), false);
}

bool ObjectPanel::PosePanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
ObjectPanel::FFDPanel::FFDPanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
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

void ObjectPanel::FFDPanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void ObjectPanel::FFDPanel::setKeyExists(bool aIsExists, bool aIsKnockable)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists && aIsKnockable);
    mGroup->setVisible(aIsExists);
}

void ObjectPanel::FFDPanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, FFD);
    const core::FFDKey::Data& data = ((const core::FFDKey*)aKey)->data();
    mEasing->setValue(data.easing(), false);
}

bool ObjectPanel::FFDPanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
ObjectPanel::ImagePanel::ImagePanel(Panel& aPanel, KeyAccessor& aAccessor,
                                    int aLabelWidth, ViaPoint& aViaPoint)
    : mAccessor(aAccessor)
    , mKnocker()
    , mGroup()
    , mBrowse()
    , mOffset()
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
    }
    setEnabled(false);
    setKeyExists(false, false);
}

void ObjectPanel::ImagePanel::knockNewKey()
{
    auto resNode = mViaPoint.requireOneResource();
    if (resNode)
    {
        this->mAccessor.knockNewImage(resNode->handle());
    }
}

void ObjectPanel::ImagePanel::setEnabled(bool aEnabled)
{
    mKnocker->setEnabled(aEnabled);
    mGroup->setEnabled(aEnabled);
}

void ObjectPanel::ImagePanel::setKeyExists(bool aIsExists, bool aIsKnockable)
{
    mKeyExists = aIsExists;
    mKnocker->setVisible(!aIsExists && aIsKnockable);
    mGroup->setVisible(aIsExists);
}

void ObjectPanel::ImagePanel::setKeyValue(const core::TimeKey* aKey)
{
    TIMEKEY_PTR_TYPE_ASSERT(aKey, Image);
    const core::ImageKey::Data& data = ((const core::ImageKey*)aKey)->data();
    mBrowse->setValue(data.resource()->identifier());
    mOffset->setValue(-data.imageOffset());
}

bool ObjectPanel::ImagePanel::keyExists() const
{
    return mKeyExists;
}

//-------------------------------------------------------------------------------------------------
ObjectPanel::ObjectPanel(ViaPoint& aViaPoint, core::Project& aProject, const QString& aTitle, QWidget* aParent)
    : Panel(aTitle, aParent)
    , mViaPoint(aViaPoint)
    , mProject(aProject)
    , mTarget()
    , mKeyAccessor()
    , mLabelWidth()
    , mAttributes()
    , mDepth()
    , mBlendMode()
    , mClipped()
    , mMovePanel()
    , mRotatePanel()
    , mScalePanel()
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

void ObjectPanel::setTarget(core::ObjectNode* aTarget)
{
    mTarget = aTarget;
    mKeyAccessor.setTarget(aTarget);

    if (mTarget)
    {
        this->setTitle(mTarget->name());
        this->show();
    }
    else
    {
        this->hide();
    }

    updateAttribute();
    updateKey();
}

void ObjectPanel::setPlayBackActivity(bool aIsActive)
{
    // resume
    if (!this->isEnabled() && !aIsActive)
    {
        updateKeyExists();
        updateKeyValue();
    }
    this->setEnabled(!aIsActive);
}

void ObjectPanel::updateKey()
{
    updateKeyExists();
    updateKeyValue();
}

void ObjectPanel::updateFrame()
{
    if (this->isEnabled())
    {
        updateKeyExists();
        updateKeyValue();
    }
}

void ObjectPanel::build()
{
    using core::Constant;

    mAttributes = new AttrGroup("Constant", mLabelWidth);
    {
        this->addGroup(mAttributes);

        // depth
        mDepth = new DecimalItem(mAttributes);
        mDepth->setRange(Constant::transMin(), Constant::transMax());
        mDepth->onValueUpdated = [=](double, double aNext)
        {
            assignDepth(this->mProject, this->mTarget, aNext);
        };
        mAttributes->addItem("depth :", mDepth);

        // blend mode
        mBlendMode = new ComboItem(mAttributes);
        for (int i = 0; i < img::BlendMode_TERM; ++i)
        {
            auto mode = (img::BlendMode)i;
            mBlendMode->box().addItem(img::getBlendNameFromBlendMode(mode));
        }
        mBlendMode->onValueUpdated = [=](int, int aNext)
        {
            assignBlendMode(this->mProject, this->mTarget, (img::BlendMode)aNext);
        };
        mAttributes->addItem("blend :", mBlendMode);

        // clipped
        mClipped = new CheckItem(mAttributes);
        mClipped->onValueUpdated = [=](bool aNext)
        {
            assignClipped(this->mProject, this->mTarget, aNext);
        };
        mAttributes->addItem("clipped :", mClipped);
    }

    mMovePanel.reset(new MovePanel(*this, mKeyAccessor, mLabelWidth));
    mRotatePanel.reset(new RotatePanel(*this, mKeyAccessor, mLabelWidth));
    mScalePanel.reset(new ScalePanel(*this, mKeyAccessor, mLabelWidth));
    mOpaPanel.reset(new OpaPanel(*this, mKeyAccessor, mLabelWidth));
    mPosePanel.reset(new PosePanel(*this, mKeyAccessor, mLabelWidth));
    mFFDPanel.reset(new FFDPanel(*this, mKeyAccessor, mLabelWidth));
    mImagePanel.reset(new ImagePanel(*this, mKeyAccessor, mLabelWidth, mViaPoint));

    this->addStretch();
}

void ObjectPanel::updateAttribute()
{
    if (mTarget)
    {
        mDepth->setValue(mTarget->depth());

        if (mTarget->renderer())
        {
            auto& renderer = *(mTarget->renderer());
            if (renderer.hasBlendMode())
            {
                mBlendMode->setItemEnabled(true);
                mBlendMode->setValue(renderer.blendMode(), false);
            }
            else
            {
                mBlendMode->setItemEnabled(false);
            }

            mClipped->setItemEnabled(true);
            mClipped->setValue(renderer.isClipped(), false);
        }
        else
        {
            mBlendMode->setItemEnabled(false);
            mClipped->setItemEnabled(false);
        }
    }
}

void ObjectPanel::updateKeyExists()
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
        mOpaPanel->setEnabled(false);
        mPosePanel->setEnabled(false);
        mFFDPanel->setEnabled(false);
        mImagePanel->setEnabled(false);
    }
}

void ObjectPanel::updateKeyValue()
{
    if (mTarget && mTarget->timeLine())
    {
        const int frame = mProject.animator().currentFrame().get();

        if (mMovePanel->keyExists())
        {
            mMovePanel->setKeyValue(mTarget->timeLine()->timeKey(core::TimeKeyType_Move, frame));
        }
        if (mRotatePanel->keyExists())
        {
            mRotatePanel->setKeyValue(mTarget->timeLine()->timeKey(core::TimeKeyType_Rotate, frame));
        }
        if (mScalePanel->keyExists())
        {
            mScalePanel->setKeyValue(mTarget->timeLine()->timeKey(core::TimeKeyType_Scale, frame));
        }
        if (mOpaPanel->keyExists())
        {
            mOpaPanel->setKeyValue(mTarget->timeLine()->timeKey(core::TimeKeyType_Opa, frame));
        }
        if (mPosePanel->keyExists())
        {
            mPosePanel->setKeyValue(mTarget->timeLine()->timeKey(core::TimeKeyType_Pose, frame));
        }
        if (mFFDPanel->keyExists())
        {
            mFFDPanel->setKeyValue(mTarget->timeLine()->timeKey(core::TimeKeyType_FFD, frame));
        }
        if (mImagePanel->keyExists())
        {
            mImagePanel->setKeyValue(mTarget->timeLine()->timeKey(core::TimeKeyType_Image, frame));
        }
    }
}

void ObjectPanel::assignDepth(core::Project& aProject, core::ObjectNode* aTarget, float aValue)
{
    XC_ASSERT(aTarget);
    if (!aTarget) return; // fail safe code

    const float prev = aTarget->depth();
    cmnd::ScopedMacro macro(aProject.commandStack(), "assign node depth");
    macro.grabListener(new ObjectNodeAttrNotifier(aProject, *aTarget));

    auto exec = [=](){ aTarget->setDepth(aValue); };
    auto undo = [=](){ aTarget->setDepth(prev); };
    aProject.commandStack().push(new cmnd::Delegatable(exec, undo));
}

void ObjectPanel::assignBlendMode(core::Project& aProject, core::ObjectNode* aTarget, img::BlendMode aValue)
{
    XC_ASSERT(aTarget);
    XC_PTR_ASSERT(aTarget->renderer());
    if (!aTarget || !aTarget->renderer()) return; // fail safe code

    auto prev = aTarget->renderer()->blendMode();
    cmnd::ScopedMacro macro(aProject.commandStack(), "assign blend mode");
    macro.grabListener(new ObjectNodeAttrNotifier(aProject, *aTarget));

    auto exec = [=](){ aTarget->renderer()->setBlendMode(aValue); };
    auto undo = [=](){ aTarget->renderer()->setBlendMode(prev); };
    aProject.commandStack().push(new cmnd::Delegatable(exec, undo));
}

void ObjectPanel::assignClipped(core::Project& aProject, core::ObjectNode* aTarget, bool aValue)
{
    XC_ASSERT(aTarget);
    XC_PTR_ASSERT(aTarget->renderer());
    if (!aTarget || !aTarget->renderer()) return; // fail safe code

    const bool prev = aTarget->renderer()->isClipped();
    cmnd::ScopedMacro macro(aProject.commandStack(), "assign node clippping flag");
    macro.grabListener(new ObjectNodeAttrNotifier(aProject, *aTarget));

    auto exec = [=](){ aTarget->renderer()->setClipped(aValue); };
    auto undo = [=](){ aTarget->renderer()->setClipped(prev); };
    aProject.commandStack().push(new cmnd::Delegatable(exec, undo));
}

} // namespace prop
} // namespace gui

